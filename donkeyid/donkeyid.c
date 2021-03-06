/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_donkeyid.h"
#include "src/donkeyid.h"
#include "php_wrapper.h"
#include <inttypes.h>
#include <main/SAPI.h>


/* If you declare any globals in php_donkeyid.h uncomment this:
 *
 * */
ZEND_DECLARE_MODULE_GLOBALS(donkeyid)


/* True global resources - no need for thread safety here */
static int le_donkeyid;
/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("donkeyid.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_donkeyid_globals, donkeyid_globals)
    STD_PHP_INI_ENTRY("donkeyid.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_donkeyid_globals, donkeyid_globals)
PHP_INI_END()
*/
/* }}} */
/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("donkeyid.type", "0", PHP_INI_SYSTEM, OnUpdateLong,dk_type,zend_donkeyid_globals,donkeyid_globals)
    STD_PHP_INI_ENTRY("donkeyid.node_id", "0", PHP_INI_SYSTEM, OnUpdateLong,dk_node_id,zend_donkeyid_globals,donkeyid_globals)
    STD_PHP_INI_ENTRY("donkeyid.epoch", "0", PHP_INI_SYSTEM, OnUpdateLong,dk_epoch,zend_donkeyid_globals,donkeyid_globals)
PHP_INI_END()
/* }}} */
//类方法参数定义
ZEND_BEGIN_ARG_INFO_EX(arginfo_donkeyid__construct, 0, 0, 0)
                ZEND_ARG_INFO(0, type)
                ZEND_ARG_INFO(0, node_id)
                ZEND_ARG_INFO(0, epoch)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_donkeyid_getIdByTime, 0, 0, 0)
                ZEND_ARG_INFO(0, time)
                ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_donkeyid_parseId, 0, 0, 0)
                ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */
/* {{{ donkeyid_functions[]
 *
 * Every user visible function must have an entry in donkeyid_functions[].
 */
const zend_function_entry donkeyid__methods[] = {
        PHP_ME(PHP_DONKEYID_CLASS_NAME, __construct, arginfo_donkeyid__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        PHP_ME(PHP_DONKEYID_CLASS_NAME, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
        PHP_ME(PHP_DONKEYID_CLASS_NAME, getNextId, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(PHP_DONKEYID_CLASS_NAME, getIdByTime, arginfo_donkeyid_getIdByTime, ZEND_ACC_PUBLIC)
        PHP_ME(PHP_DONKEYID_CLASS_NAME, parseId, arginfo_donkeyid_parseId, ZEND_ACC_PUBLIC)
        PHP_FE(dk_get_next_id, NULL)
        PHP_FE(dk_parse_id, NULL)
        PHP_FE_END    /* Must be the last line in donkeyid_functions[] */
};
/* }}} */
/* {{{ donkeyid_module_entry
 */
zend_module_entry donkeyid_module_entry = {
        STANDARD_MODULE_HEADER,
        "donkeyid",
        donkeyid__methods,
        PHP_MINIT(donkeyid),
        PHP_MSHUTDOWN(donkeyid),
        PHP_RINIT(donkeyid),        /* Replace with NULL if there's nothing to do at request start */
        PHP_RSHUTDOWN(donkeyid),    /* Replace with NULL if there's nothing to do at request end */
        PHP_MINFO(donkeyid),
        PHP_DONKEYID_VERSION,
        STANDARD_MODULE_PROPERTIES
};

/* }}} */

#ifdef COMPILE_DL_DONKEYID
ZEND_GET_MODULE(donkeyid)
#endif


/* }}} */
static long check_node_id(long type,long nodeid) {
    if (type == 0){
        if (nodeid >= NODE_ID_MASK) {
            return  NODE_ID_MASK;
        } else if(nodeid  <= 0){
            return  0;
        }
    } else if(type == 1){
        if (nodeid  >= TYPE_1_NODE_ID_MASK) {
            return TYPE_1_NODE_ID_MASK;
        } else if(nodeid  <= 0){
            return  0;
        }
    }
    return nodeid;
}


/* {{{ php_donkeyid_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_donkeyid_init_globals(zend_donkeyid_globals *donkeyid_globals)
{
	donkeyid_globals->global_value = 0;
	donkeyid_globals->global_string = NULL;
}
*/
/* }}} */

//global class
zend_class_entry *donkeyid_ce;


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION (donkeyid) {
    /* If you have INI entries, uncomment these lines
     * */
    REGISTER_INI_ENTRIES();

    zend_class_entry donkeyid_class_entry;
    INIT_CLASS_ENTRY(donkeyid_class_entry, PHP_DONKEYID_CLASS_NAME, donkeyid__methods);
    donkeyid_ce = zend_register_internal_class(&donkeyid_class_entry TSRMLS_CC);
    if (donkeyid_init() == -1){
        return FAILURE;
    }
    atexit(donkeyid_shutdown);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION (donkeyid) {
    /* uncomment this line if you have INI entries
     * */
    UNREGISTER_INI_ENTRIES();

    donkeyid_shutdown();
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION (donkeyid) {
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION (donkeyid) {
    return SUCCESS;
}
/* }}} */




/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION (donkeyid) {
    php_info_print_table_start();
    php_info_print_table_header(2, "DonkeyId support", "enabled");
    php_info_print_table_row(2, "Version", PHP_DONKEYID_VERSION);
    php_info_print_table_row(2, "Author", "zhiming.liu[email: 187231450@qq.com]");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/




/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/
PHP_METHOD (PHP_DONKEYID_CLASS_NAME, __construct) {
    long type = -1;
    long epoch = -1;
    long nodeid = -1;
    //获取类方法的参数
    if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "|lll", &type,&nodeid, &epoch) == FAILURE) {
        RETURN_FALSE;
    }
    if (type < 0){
        type = DONKEYID_G(dk_type);
    }else if(type >= MAX_DONKEYID_TYPE){
        type = 0;
    }
    zend_update_property_long(donkeyid_ce, getThis(), ZEND_STRL("type"), type TSRMLS_CC);
    if(epoch < 0){
        epoch = (time_t)DONKEYID_G(dk_epoch);
    }else if (epoch >= get_curr_timestamp()){
        epoch = 0;
    }
    zend_update_property_long(donkeyid_ce, getThis(), ZEND_STRL("epoch"), epoch TSRMLS_CC);

    if (nodeid == -1){
        nodeid = check_node_id(type,DONKEYID_G(dk_node_id));
    } else{
        nodeid = check_node_id(type,nodeid);
    }
    zend_update_property_long(donkeyid_ce, getThis(), ZEND_STRL("node_id"), nodeid TSRMLS_CC);
}

PHP_METHOD (PHP_DONKEYID_CLASS_NAME, __destruct) {
    return;
}

PHP_METHOD (PHP_DONKEYID_CLASS_NAME, getNextId) {

    char buffer[64];
    int len;
    zval *ztype = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("type"), 0 TSRMLS_CC);
    zval *znode_id = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("node_id"), 0 TSRMLS_CC);
    zval *zepoch = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("epoch"), 0 TSRMLS_CC);

    if (Z_LVAL_P(ztype) == 2){
        len = donkeyid_type_2_next_id(DONKEYID_G(dk_node_id),buffer);
        DK_RETURN_STRINGL(buffer, len, 1);
    }else{
        dk_p_t pt = {
                dtype:(int) Z_LVAL_P(ztype),
                node_id: Z_LVAL_P(znode_id),
                epoch: Z_LVAL_P(zepoch),
        };
        uint64_t donkeyid = donkeyid_next_id(pt);
        len = sprintf(buffer, "%"PRIu64, donkeyid);
        DK_RETURN_STRINGL(buffer, len, 1);
    }
}

PHP_FUNCTION(dk_get_next_id)
{
    char buffer[64];
    int len;
    long type = -1;
    //获取类方法的参数
    if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "|l", &type) == FAILURE) {
        RETURN_FALSE;
    }
    if (type < 0){
        type = DONKEYID_G(dk_type);
    }else if(type >= MAX_DONKEYID_TYPE){
        type = 0;
    }
    if (type == 2){
        len = donkeyid_type_2_next_id(DONKEYID_G(dk_node_id),buffer);
        DK_RETURN_STRINGL(buffer, len, 1);
    }else{
        dk_p_t pt = {
                dtype: (int) type,
                node_id:  DONKEYID_G(dk_node_id),
                epoch: DONKEYID_G(dk_epoch),
        };
        uint64_t donkeyid = donkeyid_next_id(pt);
        len = sprintf(buffer, "%"PRIu64, donkeyid);
        DK_RETURN_STRINGL(buffer, len, 1);
    }
}

ZEND_METHOD (PHP_DONKEYID_CLASS_NAME, getIdByTime) {

    char buffer[64];
    int len;
    char *val = NULL;
    zend_size_t val_len;
    long num;
    int n;
    //获取类方法的参数
    if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sl", &val, &val_len,&num) == FAILURE) {
        return;
    }
    uint64_t time = strtoul(val, NULL, 10);
    if (time == 0) {
        RETURN_FALSE;
    }
    zval *ztype = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("type"), 0 TSRMLS_CC);
    int type = (int) Z_LVAL_P(ztype);
    if (type == 0 ){
        if(num >= MAX_BATCH_ID_LEN){
            num = MAX_BATCH_ID_LEN;
        } else if(num <= 0){
            num = 1;
        }

    } else if (type == 1){
        if(num >= TYPE_1_SEQUENCE_MASK){
            num = TYPE_1_SEQUENCE_MASK;
        } else if(num <= 0){
            num = 1;
        }
    }
    uint64_t *idlist = (uint64_t *)malloc(sizeof(uint64_t)*num);

    zval *znode_id = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("node_id"), 0 TSRMLS_CC);
    zval *zepoch = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("epoch"), 0 TSRMLS_CC);
    dk_p_t pt = {
            dtype:(int) Z_LVAL_P(ztype),
            node_id: Z_LVAL_P(znode_id),
            epoch: Z_LVAL_P(zepoch),
    };

    if (donkeyid_get_id_by_time(idlist,time,num,pt) != 0){
        RETURN_FALSE;
    }
    array_init(return_value);
    for (n = 0; n < num ; n++) {
        len = sprintf(buffer, "%"PRIu64, *(idlist+n));
        dk_add_next_index_stringl(return_value,buffer,len,1);
    }
    free(idlist);
}

PHP_METHOD (PHP_DONKEYID_CLASS_NAME, parseId) {
    char *val = NULL;
    zend_size_t val_len;
    char buffer[64];
    int len;
    //获取类方法的参数
    if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &val, &val_len) == FAILURE) {
        return;
    }
    if (val_len > 20){
        RETURN_FALSE;
    }
    uint64_t id = strtoul(val, NULL, 10);
    if (id == 0) {
        RETURN_FALSE;
    }

    array_init(return_value);

    zval *ztype = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("type"), 0 TSRMLS_CC);
    zval *zepoch = dk_zend_read_property(donkeyid_ce, getThis(), ZEND_STRL("epoch"), 0 TSRMLS_CC);
    uint64_t time = GET_TIMESTAMP_BY_DONKEY_ID(id, Z_LVAL_P(ztype), Z_LVAL_P(zepoch));
    len = sprintf(buffer, "%"PRIu64, Z_LVAL_P(ztype) == 0 ? time : time * 1000);
    int nodeid = GET_NODE_ID_BY_DONKEY_ID(id, Z_LVAL_P(ztype));
    int sequence = GET_SEQUENCE_BY_DONKEY_ID(id, Z_LVAL_P(ztype));

    dk_add_assoc_stringl_ex(return_value,"time",5,buffer,(uint)len,1);
    add_assoc_long_ex(return_value,"node_id",8,nodeid);
    add_assoc_long_ex(return_value,"sequence",9,sequence);

}

PHP_FUNCTION(dk_parse_id)
{
    char *val = NULL;
    zend_size_t val_len;
    char buffer[64];
    int len;
    long type = -1;
    //获取类方法的参数
    if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s|l", &val, &val_len,&type) == FAILURE) {
        return;
    }
    if (type > 2 || val_len > 20){
        RETURN_FALSE;
    }
    uint64_t id = strtoul(val, NULL, 10);
    if (id == 0) {
        RETURN_FALSE;
    }
    if (type < 0){
        type = DONKEYID_G(dk_type);
    }else if(type >= MAX_DONKEYID_TYPE){
        type = 0;
    }
    array_init(return_value);
    uint64_t time = GET_TIMESTAMP_BY_DONKEY_ID(id, type, DONKEYID_G(dk_epoch));
    len = sprintf(buffer, "%"PRIu64, type == 0 ? time : time * 1000);
    int sequence = GET_SEQUENCE_BY_DONKEY_ID(id, type);
    int nodeid = GET_NODE_ID_BY_DONKEY_ID(id, type);

    dk_add_assoc_stringl_ex(return_value,"time",5,buffer,(uint)len,1);
    add_assoc_long_ex(return_value,"node_id",8,nodeid);
    add_assoc_long_ex(return_value,"sequence",9,sequence);

}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
