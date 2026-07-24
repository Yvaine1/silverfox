# ZQ28_silverfox_FreeRTOS

银狐复旦微ZQ28DR FreeRTOS工程
1.1 cd Tools:
1.2 清除文件：Python mkimg.py --clean
1.3 编译文件：Python mkimg.py --build(CA53,CR50,CR51)
1.4 编译BOOT：Python mkimg.py --boot(FSBL, CA53_BOOT)
1.5 合成Image：Python mkimg.py --img

一键编译Image:Python mkimg.py --all
单独编译某个模块：Python mkimg.py -P CA53(CA53,CA53_PT,CR50,CR50_PT,CR51,CR51_PT,BOOT,BOOT_PT,BIT28,BIT28_PT)