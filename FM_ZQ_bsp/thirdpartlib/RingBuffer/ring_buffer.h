#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// 队列深度
#define RING_BUFFER_SIZE (2048*16*16*8)
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1)  // 用于快速取模

// 检查是否是2的幂次方（环形队列大小必须是2的幂次方）
#if (RING_BUFFER_SIZE & (RING_BUFFER_SIZE - 1)) != 0
#error "RING_BUFFER_SIZE must be a power of two"
#endif

// 队列结构体
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];  // 数据缓冲区
    _Atomic uint32_t head;            // 写指针（volatile确保多线程可见性）
    _Atomic uint32_t tail;            // 读指针（volatile确保多线程可见性）
} ring_buffer_t;

/**
 * @brief 初始化环形队列
 * @param rb 队列指针
 */
void ring_buffer_init(ring_buffer_t *rb);

/**
 * @brief 检查队列是否为空
 * @param rb 队列指针
 * @return true-空, false-非空
 */
bool ring_buffer_is_empty(const ring_buffer_t *rb);

/**
 * @brief 检查队列是否已满
 * @param rb 队列指针
 * @return true-满, false-未满
 */
bool ring_buffer_is_full(const ring_buffer_t *rb);

/**
 * @brief 获取队列中数据数量
 * @param rb 队列指针
 * @return 数据数量
 */
size_t ring_buffer_size(const ring_buffer_t *rb);

/**
 * @brief 获取队列空闲空间
 * @param rb 队列指针
 * @return 空闲空间大小
 */
size_t ring_buffer_free_space(const ring_buffer_t *rb);

/**
 * @brief 向队列写入一个字节（线程安全）
 * @param rb 队列指针
 * @param data 要写入的数据
 * @return true-成功, false-队列已满
 */
bool ring_buffer_put(ring_buffer_t *rb, uint8_t data);

/**
 * @brief 从队列读取一个字节（线程安全）
 * @param rb 队列指针
 * @param data 读取数据的存储位置
 * @return true-成功, false-队列为空
 */
bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data);

/**
 * @brief 向队列写入多个字节（线程安全）
 * @param rb 队列指针
 * @param data 要写入的数据指针
 * @param length 要写入的数据长度
 * @return 实际写入的字节数
 */
size_t ring_buffer_put_bulk(ring_buffer_t *rb, const uint8_t *data, size_t length);

/**
 * @brief 从队列读取多个字节（线程安全）
 * @param rb 队列指针
 * @param data 读取数据的存储位置
 * @param length 要读取的数据长度
 * @return 实际读取的字节数
 */
size_t ring_buffer_get_bulk(ring_buffer_t *rb, uint8_t *data, size_t length);

/**
 * @brief 查看队列下一个字节（不移动读指针）
 * @param rb 队列指针
 * @param data 查看数据的存储位置
 * @return true-成功, false-队列为空
 */
bool ring_buffer_peek(const ring_buffer_t *rb, uint8_t *data);

/**
 * @brief 丢弃队列中的指定数量字节
 * @param rb 队列指针
 * @param count 要丢弃的字节数
 * @return 实际丢弃的字节数
 */
size_t ring_buffer_discard(ring_buffer_t *rb, size_t count);

#ifdef __cplusplus
}
#endif

#endif // RING_BUFFER_H