#ifndef _FMSH_FATFS_EXAMPLE_H_
#define _FMSH_FATFS_EXAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int wr_test_example(char *host_path, char *dst_path);

#define FWRITE_READ_BUFFER_SIZE_MAX (1 * 1024 * 1024)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
