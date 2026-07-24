/**
 * @file shell_cfg.h
 * @author Letter (nevermindzzt@gmail.com)
 * @brief shell config
 * @version 3.0.0
 * @date 2019-12-31
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#ifndef __SHELL_CFG_H__
#define __SHELL_CFG_H__

#ifdef SHELL_CFG_USER
#include SHELL_CFG_USER
#endif

#ifndef SHELL_TASK_WHILE
/**
 * @brief 鏄惁浣跨敤榛樿shell浠诲姟while寰幆
 *        浣胯兘姝ゅ畯锛屽垯`shellTask()`鍑芥暟浼氫竴鐩村惊鐜鍙栬緭鍏ワ紝涓€鑸娇鐢ㄦ搷浣滅郴缁熷缓绔媠hell
 *        浠诲姟鏃朵娇鑳芥瀹忥紝鍏抽棴姝ゅ畯鐨勬儏鍐典笅锛屼竴鑸€傜敤浜庢棤鎿嶄綔绯荤粺锛屽湪涓诲惊鐜腑璋冪敤`shellTask()`
 */
#define     SHELL_TASK_WHILE            1
#endif /** SHELL_TASK_WHILE */

#ifndef SHELL_USING_CMD_EXPORT
/**
 * @brief 鏄惁浣跨敤鍛戒护瀵煎嚭鏂瑰紡
 *        浣胯兘姝ゅ畯鍚庯紝鍙互浣跨敤`SHELL_EXPORT_CMD()`绛夊鍑哄懡浠�
 *        瀹氫箟shell鍛戒护锛屽叧闂瀹忕殑鎯呭喌涓嬶紝闇€瑕佷娇鐢ㄥ懡浠よ〃鐨勬柟寮�
 */
#define     SHELL_USING_CMD_EXPORT      1
#endif /** SHELL_USING_CMD_EXPORT */

#ifndef SHELL_USING_COMPANION
/**
 * @brief 鏄惁浣跨敤shell浼寸敓瀵硅薄
 *        涓€浜涙墿灞曠殑缁勪欢(鏂囦欢绯荤粺鏀寔锛屾棩蹇楀伐鍏风瓑)闇€瑕佷娇鐢ㄤ即鐢熷璞�
 */
#define     SHELL_USING_COMPANION       0
#endif /** SHELL_USING_COMPANION */

#ifndef SHELL_SUPPORT_END_LINE
/**
 * @brief 鏀寔shell灏捐妯″紡
 */
#define     SHELL_SUPPORT_END_LINE      0
#endif /** SHELL_SUPPORT_END_LINE */

#ifndef SHELL_HELP_LIST_USER
/**
 * @brief 鏄惁鍦ㄨ緭鍑哄懡浠ゅ垪琛ㄤ腑鍒楀嚭鐢ㄦ埛
 */
#define     SHELL_HELP_LIST_USER        0
#endif /** SHELL_HELP_LIST_USER */

#ifndef SHELL_HELP_LIST_VAR
/**
 * @brief 鏄惁鍦ㄨ緭鍑哄懡浠ゅ垪琛ㄤ腑鍒楀嚭鍙橀噺
 */
#define     SHELL_HELP_LIST_VAR         0
#endif /** SHELL_HELP_LIST_VAR */

#ifndef SHELL_HELP_LIST_KEY
/**
 * @brief 鏄惁鍦ㄨ緭鍑哄懡浠ゅ垪琛ㄤ腑鍒楀嚭鎸夐敭
 */
#define     SHELL_HELP_LIST_KEY         0
#endif /** SHELL_HELP_LIST_KEY */

#ifndef SHELL_HELP_SHOW_PERMISSION
/**
 * @brief 鏄惁鍦ㄨ緭鍑哄懡浠ゅ垪琛ㄤ腑灞曠ず鍛戒护鏉冮檺
 */
#define     SHELL_HELP_SHOW_PERMISSION  1
#endif /** SHELL_HELP_SHOW_PERMISSION */

#ifndef SHELL_ENTER_LF
/**
 * @brief 浣跨敤LF浣滀负鍛戒护琛屽洖杞﹁Е鍙�
 *        鍙互鍜孲HELL_ENTER_CR鍚屾椂寮€鍚�
 */
#define     SHELL_ENTER_LF              1
#endif /** SHELL_ENTER_LF */

#ifndef SHELL_ENTER_CR
/**
 * @brief 浣跨敤CR浣滀负鍛戒护琛屽洖杞﹁Е鍙�
 *        鍙互鍜孲HELL_ENTER_LF鍚屾椂寮€鍚�
 */
#define     SHELL_ENTER_CR              1
#endif /** SHELL_ENTER_CR */

#ifndef SHELL_ENTER_CRLF
/**
 * @brief 浣跨敤CRLF浣滀负鍛戒护琛屽洖杞﹁Е鍙�
 *        涓嶅彲浠ュ拰SHELL_ENTER_LF鎴朣HELL_ENTER_CR鍚屾椂寮€鍚�
 */
#define     SHELL_ENTER_CRLF            0
#endif /** SHELL_ENTER_CRLF */

#ifndef SHELL_EXEC_UNDEF_FUNC
/**
 * @brief 浣跨敤鎵ц鏈鍑哄嚱鏁扮殑鍔熻兘
 *        鍚敤鍚庯紝鍙互閫氳繃`exec [addr] [args]`鐩存帴鎵ц瀵瑰簲鍦板潃鐨勫嚱鏁�
 * @attention 濡傛灉鍦板潃閿欒锛屽彲鑳戒細鐩存帴寮曡捣绋嬪簭宕╂簝
 */
#define     SHELL_EXEC_UNDEF_FUNC       0
#endif /** SHELL_EXEC_UNDEF_FUNC */

#ifndef SHELL_PARAMETER_MAX_NUMBER
/**
 * @brief shell鍛戒护鍙傛暟鏈€澶ф暟閲�
 *        鍖呭惈鍛戒护鍚嶅湪鍐咃紝瓒呰繃16涓弬鏁板苟涓斾娇鐢ㄤ簡鍙傛暟鑷姩杞崲鐨勬儏鍐典笅锛岄渶瑕佷慨鏀规簮鐮�
 */
#define     SHELL_PARAMETER_MAX_NUMBER  8
#endif /** SHELL_PARAMETER_MAX_NUMBER */

#ifndef SHELL_HISTORY_MAX_NUMBER
/**
 * @brief 鍘嗗彶鍛戒护璁板綍鏁伴噺
 */
#define     SHELL_HISTORY_MAX_NUMBER    5
#endif /** SHELL_HISTORY_MAX_NUMBER */

#ifndef SHELL_DOUBLE_CLICK_TIME
/**
 * @brief 鍙屽嚮闂撮殧(ms)
 *        浣胯兘瀹廯SHELL_LONG_HELP`鍚庢瀹忕敓鏁堬紝瀹氫箟鍙屽嚮tab琛ュ叏help鐨勬椂闂撮棿闅�
 */
#define     SHELL_DOUBLE_CLICK_TIME     200
#endif /** SHELL_DOUBLE_CLICK_TIME */

#ifndef SHELL_QUICK_HELP
/**
 * @brief 蹇€熷府鍔�
 *        浣滅敤浜庡弻鍑籺ab鐨勫満鏅紝褰撲娇鑳芥瀹忔椂锛屽弻鍑籺ab涓嶄細瀵瑰懡浠よ繘琛宧elp琛ュ叏锛岃€屾槸鐩存帴鏄剧ず瀵瑰簲鍛戒护鐨勫府鍔╀俊鎭�
 */
#define     SHELL_QUICK_HELP            1
#endif /** SHELL_QUICK_HELP */

#ifndef SHELL_KEEP_RETURN_VALUE
/**
 * @brief 淇濆瓨鍛戒护杩斿洖鍊�
 *        寮€鍚悗浼氶粯璁ゅ畾涔変竴涓猔RETVAL`鍙橀噺锛屼細淇濆瓨涓婁竴娆″懡浠ゆ墽琛岀殑杩斿洖鍊硷紝鍙互鍦ㄩ殢鍚庣殑鍛戒护涓繘琛岃皟鐢�
 *        濡傛灉鍛戒护鐨刞SHELL_CMD_DISABLE_RETURN`鏍囧織琚缃紝鍒欒鍛戒护涓嶄細鏇存柊`RETVAL`
 */
#define     SHELL_KEEP_RETURN_VALUE     0
#endif /** SHELL_KEEP_RETURN_VALUE */

#ifndef SHELL_MAX_NUMBER
/**
 * @brief 绠＄悊鐨勬渶澶hell鏁伴噺
 */
#define     SHELL_MAX_NUMBER            5
#endif /** SHELL_MAX_NUMBER */

#ifndef SHELL_PRINT_BUFFER
/**
 * @brief shell鏍煎紡鍖栬緭鍑虹殑缂撳啿澶у皬
 *        涓�0鏃朵笉浣跨敤shell鏍煎紡鍖栬緭鍑�
 */
#define     SHELL_PRINT_BUFFER          128
#endif /** SHELL_PRINT_BUFFER */

#ifndef SHELL_SCAN_BUFFER
/**
 * @brief shell鏍煎紡鍖栬緭鍏ョ殑缂撳啿澶у皬
 *        涓�0鏃朵笉浣跨敤shell鏍煎紡鍖栬緭鍏�
 * @note shell鏍煎紡鍖栬緭鍏ヤ細闃诲shellTask, 浠呴€傜敤浜庡湪鏈夋搷浣滅郴缁熺殑鎯呭喌涓嬩娇鐢�
 */
#define     SHELL_SCAN_BUFFER          0
#endif /** SHELL_SCAN_BUFFER */

#ifndef SHELL_GET_TICK
/**
 * @brief 鑾峰彇绯荤粺鏃堕棿(ms)
 *        瀹氫箟姝ゅ畯涓鸿幏鍙栫郴缁烼ick锛屽`HAL_GetTick()`
 * @note 姝ゅ畯涓嶅畾涔夋椂鏃犳硶浣跨敤鍙屽嚮tab琛ュ叏鍛戒护help锛屾棤娉曚娇鐢╯hell瓒呮椂閿佸畾
 */
#define     SHELL_GET_TICK()            0
#endif /** SHELL_GET_TICK */

#ifndef SHELL_USING_LOCK
/**
 * @brief 浣跨敤閿�
 * @note 浣跨敤shell閿佹椂锛岄渶瑕佸鍔犻攣鍜岃В閿佽繘琛屽疄鐜�
 */
#define     SHELL_USING_LOCK            0
#endif /** SHELL_USING_LOCK */

#ifndef SHELL_MALLOC
/**
 * @brief shell鍐呭瓨鍒嗛厤
 *        shell鏈韩涓嶉渶瑕佹鎺ュ彛锛岃嫢浣跨敤shell浼寸敓瀵硅薄锛岄渶瑕佽繘琛屽畾涔�
 */
#define     SHELL_MALLOC(size)          0
#endif /** SHELL_MALLOC */

#ifndef SHELL_FREE
/**
 * @brief shell鍐呭瓨閲婃斁
 *        shell鏈韩涓嶉渶瑕佹鎺ュ彛锛岃嫢浣跨敤shell浼寸敓瀵硅薄锛岄渶瑕佽繘琛屽畾涔�
 */
#define     SHELL_FREE(obj)             0
#endif /** SHELL_FREE */

#ifndef SHELL_SHOW_INFO
/**
 * @brief 鏄惁鏄剧ずshell淇℃伅
 */
#define     SHELL_SHOW_INFO             0
#endif /** SHELL_SHOW_INFO */

#ifndef SHELL_CLS_WHEN_LOGIN
/**
 * @brief 鏄惁鍦ㄧ櫥褰曞悗娓呴櫎鍛戒护琛�
 */
#define     SHELL_CLS_WHEN_LOGIN        1
#endif /** SHELL_CLS_WHEN_LOGIN */

#ifndef SHELL_DEFAULT_USER
/**
 * @brief shell榛樿鐢ㄦ埛
 */
#define     SHELL_DEFAULT_USER          "letter"
#endif /** SHELL_DEFAULT_USER */

#ifndef SHELL_DEFAULT_USER_PASSWORD
/**
 * @brief shell榛樿鐢ㄦ埛瀵嗙爜
 *        鑻ラ粯璁ょ敤鎴蜂笉闇€瑕佸瘑鐮侊紝璁句负""
 */
#define     SHELL_DEFAULT_USER_PASSWORD ""
#endif /** SHELL_DEFAULT_USER_PASSWORD */

#ifndef SHELL_LOCK_TIMEOUT
/**
 * @brief shell鑷姩閿佸畾瓒呮椂
 *        shell褰撳墠鐢ㄦ埛瀵嗙爜鏈夋晥鐨勬椂鍊欑敓鏁堬紝瓒呮椂鍚庝細鑷姩閲嶆柊閿佸畾shell
 *        璁剧疆涓�0鏃跺叧闂嚜鍔ㄩ攣瀹氬姛鑳斤紝鏃堕棿鍗曚綅涓篳SHELL_GET_TICK()`鍗曚綅
 * @note 浣跨敤瓒呮椂閿佸畾蹇呴』淇濊瘉`SHELL_GET_TICK()`鏈夋晥
 */
#define     SHELL_LOCK_TIMEOUT          0 * 60 * 1000
#endif /** SHELL_LOCK_TIMEOUT */

#ifndef SHELL_USING_FUNC_SIGNATURE
/**
 * @brief 浣跨敤鍑芥暟绛惧悕
 *        浣胯兘鍚庯紝鍙互鍦ㄥ０鏄庡懡浠ゆ椂锛屾寚瀹氬嚱鏁扮殑绛惧悕锛宻hell 浼氭牴鎹嚱鏁扮鍚嶈繘琛屽弬鏁拌浆鎹紝
 *        鑰屼笉鏄嚜鍔ㄥ垽鏂弬鏁扮殑绫诲瀷锛屽鏋滃弬鏁板拰鍑芥暟绛惧悕涓嶅尮閰嶏紝浼氬仠姝㈡墽琛屽懡浠�
 */
#define     SHELL_USING_FUNC_SIGNATURE  0
#endif /** SHELL_USING_FUNC_SIGNATURE */

#ifndef SHELL_SUPPORT_ARRAY_PARAM
/**
 * @brief 鏀寔鏁扮粍鍙傛暟
 *        浣胯兘鍚庯紝鍙互鍦ㄥ懡浠や腑浣跨敤鏁扮粍鍙傛暟锛屽`cmd [1,2,3]`
 *        闇€瑕佷娇鑳� `SHELL_USING_FUNC_SIGNATURE` 瀹忥紝骞朵笖閰嶇疆 `SHELL_MALLOC`, `SHELL_FREE`
 */
#define     SHELL_SUPPORT_ARRAY_PARAM   0
#endif /** SHELL_SUPPORT_ARRAY_PARAM */

#endif
