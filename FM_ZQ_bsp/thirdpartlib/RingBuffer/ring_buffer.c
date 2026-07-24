#include "ring_buffer.h"
#include <string.h>

// 内存屏障宏（确保指令执行顺序）
#if defined(__ICCARM__)
#define	MEMORY_BARRIER() __asm volatile("dmb sy")
#elif defined(__GNUC__) || defined(__clang__)
#define MEMORY_BARRIER() __sync_synchronize()
#elif defined(_MSC_VER)
#define MEMORY_BARRIER() _ReadWriteBarrier()
#else
#define MEMORY_BARRIER() ((void)0)
#endif

// 原子加载（确保读取最新值）
//static inline uint32_t atomic_load(const volatile uint32_t *ptr) {
//    MEMORY_BARRIER();
//    return *ptr;
//}

// 原子存储（确保写入立即可见）
//static inline void atomic_store(volatile uint32_t *ptr, uint32_t value) {
//    *ptr = value;
//    MEMORY_BARRIER();
//}

void ring_buffer_init(ring_buffer_t *rb) {
    if (rb == NULL) return;
    
    memset(rb->buffer, 0, sizeof(rb->buffer));
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
}

bool ring_buffer_is_empty(const ring_buffer_t *rb) {
    if (rb == NULL) return true;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    return (head == tail);
}

bool ring_buffer_is_full(const ring_buffer_t *rb) {
    if (rb == NULL) return true;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    return ((head - tail) == RING_BUFFER_SIZE);
}

size_t ring_buffer_size(const ring_buffer_t *rb) {
    if (rb == NULL) return 0;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    return (head >= tail)?(head - tail):(UINT32_MAX - tail + head + 1);
}

size_t ring_buffer_free_space(const ring_buffer_t *rb) {
    if (rb == NULL) return 0;
    
    return RING_BUFFER_SIZE - ring_buffer_size(rb);
}

bool ring_buffer_put(ring_buffer_t *rb, uint8_t data) {
    if (rb == NULL) return false;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    // 检查队列是否已满
    if ((head - tail) == RING_BUFFER_SIZE) {
        return false;
    }
    
    // 写入数据
	MEMORY_BARRIER();
    rb->buffer[head & RING_BUFFER_MASK] = data;
    
    // 更新写指针（确保数据先写入，再更新指针）
    MEMORY_BARRIER();
    atomic_store(&rb->head, head + 1);
    
    return true;
}

bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data) {
    if (rb == NULL || data == NULL) return false;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    // 检查队列是否为空
    if (head == tail) {
        return false;
    }
    
    // 读取数据
	MEMORY_BARRIER();
    *data = rb->buffer[tail & RING_BUFFER_MASK];
    
    // 更新读指针（确保数据先读取，再更新指针）
    MEMORY_BARRIER();
    atomic_store(&rb->tail, tail + 1);
    
    return true;
}

size_t ring_buffer_put_bulk(ring_buffer_t *rb, const uint8_t *data, size_t length) {
    if (rb == NULL || data == NULL || length == 0) return 0;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    size_t free_space = RING_BUFFER_SIZE - (head - tail);
    if (free_space == 0) return 0;
    
    // 计算实际可以写入的数据量
    size_t bytes_to_write = (length > free_space) ? free_space : length;
    
    // 计算写位置和连续空间
    uint32_t write_index = head & RING_BUFFER_MASK;
    size_t continuous_space = RING_BUFFER_SIZE - write_index;
    
    if (bytes_to_write <= continuous_space) {
        // 一次写入即可
        memcpy(&rb->buffer[write_index], data, bytes_to_write);
    } else {
        // 需要分两次写入（跨越缓冲区末尾）
        memcpy(&rb->buffer[write_index], data, continuous_space);
        memcpy(&rb->buffer[0], data + continuous_space, bytes_to_write - continuous_space);
    }
    
    // 更新写指针
    MEMORY_BARRIER();
    atomic_store(&rb->head, head + bytes_to_write);
    
    return bytes_to_write;
}

size_t ring_buffer_get_bulk(ring_buffer_t *rb, uint8_t *data, size_t length) {
    if (rb == NULL || data == NULL || length == 0) return 0;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    size_t available_data = head - tail;
    if (available_data == 0) return 0;
    
    // 计算实际可以读取的数据量
    size_t bytes_to_read = (length > available_data) ? available_data : length;
    
    // 计算读位置和连续数据
    uint32_t read_index = tail & RING_BUFFER_MASK;
    size_t continuous_data = RING_BUFFER_SIZE - read_index;
    
    if (bytes_to_read <= continuous_data) {
        // 一次读取即可
        memcpy(data, &rb->buffer[read_index], bytes_to_read);
    } else {
        // 需要分两次读取（跨越缓冲区末尾）
        memcpy(data, &rb->buffer[read_index], continuous_data);
        memcpy(data + continuous_data, &rb->buffer[0], bytes_to_read - continuous_data);
    }
    
    // 更新读指针
    MEMORY_BARRIER();
    atomic_store(&rb->tail, tail + bytes_to_read);
    
    return bytes_to_read;
}

bool ring_buffer_peek(const ring_buffer_t *rb, uint8_t *data) {
    if (rb == NULL || data == NULL) return false;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    // 检查队列是否为空
    if (head == tail) {
        return false;
    }
    
    // 读取数据（不更新读指针）
    *data = rb->buffer[tail & RING_BUFFER_MASK];
    
    return true;
}

size_t ring_buffer_discard(ring_buffer_t *rb, size_t count) {
    if (rb == NULL || count == 0) return 0;
    
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    
    size_t available_data = head - tail;
    if (available_data == 0) return 0;
    
    // 计算实际可以丢弃的数据量
    size_t bytes_to_discard = (count > available_data) ? available_data : count;
    
    // 更新读指针
    MEMORY_BARRIER();
    atomic_store(&rb->tail, tail + bytes_to_discard);
    
    return bytes_to_discard;
}