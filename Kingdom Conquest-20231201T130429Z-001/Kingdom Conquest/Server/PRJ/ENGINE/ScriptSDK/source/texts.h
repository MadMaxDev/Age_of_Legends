#ifndef __PS_TEXT_H__
#define __PS_TEXT_H__

#ifdef LANGUAGE_CHINESE
	// Compiler messages
	#define TXT_s_ALREADY_DECLARED				 PS_T("'%s' 已经声明过了.")
	#define TXT_ARG_NOT_LVALUE					 PS_T("赋值参数不是左值")

	#define TXT_CANT_IMPLICITLY_CONVERT_s_TO_s  PS_T("不能够隐含的将 '%s' 转化为 '%s'.")
	#define TXT_CANT_RETURN_VALUE               PS_T("返回类型为'void'时不能返回值")
	#define TXT_CHANGE_SIGN_u_d                 PS_T("隐含转换改变了值的符号, %u -> %d.")
	#define TXT_CHANGE_SIGN_d_u                 PS_T("隐含转换改变了值的符号, %d -> %u.")
	#define TXT_COMPILING_s                     PS_T("编译 %s")
	#define TXT_CONST_NOT_PRIMITIVE             PS_T("只有基本类型能被声明为'const'")

	#define TXT_DATA_TYPE_CANT_BE_s             PS_T("数据类型不能为 '%s'")
	#define TXT_DEFAULT_MUST_BE_LAST            PS_T("'default'必需在所有'case'的后面")

	#define TXT_EXPECTED_s                      PS_T("期望出现 '%s'")
	#define TXT_EXPECTED_CONSTANT               PS_T("期望出现常数")
	#define TXT_ARRAY_SIZE_MUST_BE_INT_CONSTANT PS_T("数组大小必需是整型常量")
	#define TXT_CONCATE_STRING_TYPE_NOT_MATCH   PS_T("字符串连接时类型不匹配")
	// TODO: Should be TXT_NO_CONVERSION
	#define TXT_EXPECTED_CONSTANT_s_FOUND_s   PS_T("期望出现常数类型 '%s', 却发现了 '%s'")
	#define TXT_EXPECTED_DATA_TYPE            PS_T("期望出现数据类型")
	#define TXT_EXPECTED_EXPRESSION_VALUE     PS_T("期望出现表达式")
	#define TXT_EXPECTED_IDENTIFIER           PS_T("期望出现标识符")
	#define TXT_EXPECTED_ONE_OF               PS_T("期望出现其中的一个: ")
	#define TXT_EXPECTED_OPERATOR             PS_T("期望出现操作符")
	#define TXT_EXPECTED_s_OR_s               PS_T("期望出现 %s' 或 '%s'")
	#define TXT_EXPECTED_POST_OPERATOR        PS_T("期望出现后置操作符")
	#define TXT_EXPECTED_PRE_OPERATOR         PS_T("期望出现前置操作符")
	#define TXT_EXPECTED_STRING               PS_T("期望出现字符串")
	#define TXT_EXPR_MUST_BE_BOOL             PS_T("表达式必需为'bool'类型")
	#define TXT_EXPR_MUST_EVAL_TO_CONSTANT    PS_T("表达式必需为一个常数值")
	#define TXT_MACRO_s_REDEFINITION		  PS_T("宏 '%s' 重定义")
	#define	TXT_MACRO_s_IS_RESREVED			  PS_T("宏 '%s'被编译器保留, 忽略")
	#define TXT_UNEXPECTED_NUMBERELSE		  PS_T("不期望出现的 #else")
	#define TXT_UNEXPECTED_NUMBERENDIF		  PS_T("不期望出现的 #endif")

	#define TXT_INCLUDE_FILE_TOO_MANY_MAX_d	  PS_T("包含的文件太多: 深度最大为: %d")
	#define TXT_INCLUDE_EXPECT_FILENAME		  PS_T("在'include'后期望出现一个文件名, 但发现了换行")
	#define TXT_CANT_READ_FILE_s			  PS_T("不能读取文件'%s'")

	#define TXT_IFDEF_EXPECTED_IDENTIFIER	  PS_T("#if[n]def 期望出现一个标识符, 但发现了换行符")
	#define TXT_UNDEF_EXPECTED_IDENTIFIER	  PS_T("#undef 期望出现一个标识符, 但发现了换行符")

	#define TXT_FUNCTION_s_ALREADY_EXIST      PS_T("已经有一个同名称和类型的函数'%s'存在")

	#define TXT_IDENTIFIER_s_NOT_DATA_TYPE    PS_T("标识符 '%s' 并不是一个数据类型")
	#define TXT_ILLEGAL_CALL                  PS_T("无效的函数调用")
	// TODO: Should be TXT_ILLEGAL_OPERATION_ON_s
	#define TXT_ILLEGAL_OPERATION             PS_T("非法的操作符")
	#define TXT_ILLEGAL_OPERATION_ON_s        PS_T("对 '%s' 的操作符非法")

	#define TXT_INVALID_BREAK                 PS_T("无效的 'break'")
	#define TXT_INVALID_CONTINUE              PS_T("无效的 'continue'")
	#define TXT_INVALID_TYPE                  PS_T("无效的类型")
	#define TXT_INVALID_OPERATOR			  PS_T("无效的操作")
	#define TXT_ILLEGAL_VARIABLE_NAME_s       PS_T("不合法的的变量名 '%s',已经有一个类占用了这个名称.")

	#define TXT_MORE_THAN_ONE_MATCHING_OP     PS_T("找到了多个匹配的重载操作符")
	#define TXT_NO_MATCHING_OP				  PS_T("没有找到重载操作")

	#define TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s PS_T("找到与'%s' 有多个不同的匹配的函数")
	#define TXT_MUST_BE_OBJECT					  PS_T("只有类对象才有构造函数")
	#define TXT_s_MUST_BE_SENT_BY_REF			  PS_T("'%s' 必需以引用类型传递")
	#define TXT_MUST_RETURN_VALUE				  PS_T("函数必需返回值")

	#define TXT_NAME_CONFILIC_s_OBJ_EXIST		PS_T("名字冲突. 名称为'%s'的类已经定义")
	#define TXT_NAME_CONFLICT_s_CONSTANT        PS_T("名字冲突. '%s' 是一个常数.")
	#define TXT_NAME_CONFLICT_s_EXTENDED_TYPE   PS_T("名字冲突. '%s' 是一个扩展的数据类型名(或类名).")
	#define TXT_NAME_CONFLICT_s_FUNCTION        PS_T("名字冲突. '%s' 是一个函数.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_VAR      PS_T("名字冲突. '%s' 是一个全局变量.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY PS_T("名字冲突. '%s' 是一个全局属性.")
	#define TXT_NAME_CONFILIC_s_ENUM_CONSTANT	PS_T("名字冲突. '%s' 是一个枚举常量.")
	#define TXT_NAME_CONFLICT_s_OBJ_PROPERTY    PS_T("名字冲突. '%s' 是一个对象属性.")
	#define TXT_NAME_CONFLICT_s_OBJ_METHOD      PS_T("名字冲突. '%s' 是一个对象方法.")
	#define TXT_NAME_CONFLICT_s_SYSTEM_FUNCTION PS_T("名字冲突. '%s' 是一个系统函数.")

	#define TXT_NO_CONVERSION_s_TO_s            PS_T("没有从 '%s' 到 '%s' 的转换.")
	#define TXT_NO_CONVERSION_s_TO_MATH_TYPE    PS_T("没有从 '%s' 到 数值类型的转换.")
	#define TXT_NO_DEFAULT_COPY_OP              PS_T("这个数据类型没有一个有效的拷贝函数.")
	#define TXT_NO_MATCHING_SIGNATURES_TO_s     PS_T("没有找到与 '%s' 匹配的函数")
	#define TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s PS_T("没有找到匹配的重载操作符'%s'函数, 类型 '%s' ")
	#define TXT_NOT_ALL_PATHS_RETURN            PS_T("并不是所有路径都有返回值")
	#define TXT_s_NOT_AVAILABLE_FOR_s           PS_T("'%s' 对 '%s'无效")
	#define TXT_s_NOT_DECLARED                  PS_T("'%s' 没有声明")
	#define TXT_NOT_EXACT_g_d_g                 PS_T("数值的隐含转换并不精确, %g -> %d -> %g.")
	#define TXT_NOT_EXACT_d_g_d                 PS_T("数值的隐含转换并不精确, %d -> %g -> %d.")
	#define TXT_NOT_EXACT_g_u_g                 PS_T("数值的隐含转换并不精确, %g -> %u -> %g.")
	#define TXT_NOT_EXACT_u_g_u                 PS_T("数值的隐含转换并不精确, %u -> %g -> %u.")

	#define TXT_CONVERT_s_TO_s_MAY_LOST_DATA    PS_T("从 '%s' 转换到 '%s', 可能丢失数据")

	#define TXT_CANT_CONVERT_s_TO_s			    PS_T("不能从 '%s' 转换到 '%s' ")
	#define TXT_CONVERT_s_TO_s_IS_DANGEROUS		PS_T("从 '%s' 转换到 '%s', 可能很危险" )

	#define TXT_REF_TYPE_CANNOT_BE_ARRAY	    PS_T("引用类型不能为一个数组")
	#define TXT_REF_s_INITEXPR_TYPE_ERROR	    PS_T("引用变量初始化表达式的类型不正确!")
	#define TXT_REF_s_MUST_BE_INITIALIZED       PS_T("引用 '%s' 必需初始化")
	#define TXT_REF_s_CAN_NOT_BE_TEMPOARY	    PS_T("引用 '%s' 不能为临时对象")
	#define TXT_REF_CANNOT_BE_TEMPORARY		    PS_T("引用不能为临时的")

	#define TXT_INIT_ARRAY_SIZE_d_DONOT_MATCH_d	PS_T("初始化数组大小(%d)与声明的大小(%d)不匹配")
	#define TXT_VARIABLE_INITEXPR_TYPE_ERROR    PS_T("变量初始化表达式的大小不正确")
	#define TXT_CHAR_INIT_EXPR_TOO_FEW		    PS_T("字符型常量初始化的字符个数太少")
	#define TXT_CHAR_INIT_EXPR_TOO_MANY		    PS_T("字符型常量初始化的字符个数太多")

	#define TXT_ARRAY_SIZE_ERROR			    PS_T("数组的大小必需为大于1的无符号整型常数!")
	#define TXT_ARRAY_DIM_TOO_LARGE			    PS_T("数组的维数太大")
	#define TXT_ARRAY_SIZE_TOO_LARGE		    PS_T("数组的大小太大")
	#define TXT_ASSIGN_LEFT_MUSTBE_LVALUE	    PS_T("赋值表达式的左边必须是左值"	)
	#define TXT_ARRAY_INDEX_MUSTBE_INTEGER	    PS_T("数组的下标一定要为整数")
	#define	TXT_REF_s_TYPE_IS_ERROR			    PS_T("引用'%s'的类型出错!")
	#define TXT_INAVLID_GET_s_POINTER		    PS_T("对类型'%s'的取地址操作无效")
	#define TXT_VAR_ARG_s_DONT_SUPPORT		    PS_T("类型'%s'不支持作为变参函数的参数")
	#define TXT_VAR_FIRST_ARG_MUST_BE_TEXT	    PS_T("变参函数的第一个参数必需为文字常量或者整型常数.")
	#define TXT_VAR_FUNC_ARGSIZE_TOO_SMALL	    PS_T("变参函数的参数个数太少(至少为一个)")

	#define TXT_s_NOT_FUNCTION                  PS_T("没有找到函数 '%s'")
	#define TXT_s_NOT_INITIALIZED               PS_T("'%s' 没有始化.")
	#define TXT_s_NOT_MEMBER_OF_s               PS_T("'%s' 不是 '%s' 的成员")
	#define TXT_NOT_VALID_REFERENCE             PS_T("无效的引用")

	#define TXT_PROPERTY_s_ALREADY_EXIST	    PS_T("属性'%s'已经存在")
	#define TXT_PROPERTY_CANNOT_BE_REF			PS_T("类成员属性不能为引用类型")
	#define TXT_CONSTRUCTOR_FUNC_NAME_ERROR	    PS_T("构造函数的名称必需和类名相同")
	#define TXT_DESTRUCTOR_FUNC_NAME_ERROR	    PS_T("析构函数的名称必需和类名相同")
	#define TXT_DESTRUCTOR_ALREADY_EXIST	    PS_T("析构函数已经存在")
	#define TXT_DESTRUCTOR_DONT_HAS_ARGS	    PS_T("析构函数不能有参数")
	#define TXT_METHOD_s_ALREADY_EXIST		    PS_T("类方法'%s'已经存在")
	#define TXT_CONSTRUCTOR_s_ALREADY_EXIST	    PS_T("构造函数'%s'已经存在")
	#define TXT_OPOVERLOAD_s_ALREADY_EXIST      PS_T("对操作符'%s'的重载已经存在")
	#define TXT_OPOVERLOAD_s_ONLY_HAS_TWOPARAM	PS_T("重载操作符'%s'的参数只有两个")
	#define TXT_OPOVERLOAD_s_HAS_TOOMANY_PARAM	PS_T("重载操作符'%s'的参数太多")
	#define TXT_OP_s_NOT_ALLOW_OVERLOAD			PS_T("操作符'%s'不允许被重载")

	#define TXT_METHODNAME_s_SAMEAS_CLASSNAME   PS_T("方法'%s'与类名重名")
	#define TXT_INVALID_THIS_POINTER		    PS_T("无效的this指针")
	#define	TXT_OBJECT_s_CANNOT_BE_INSTANCE     PS_T("类'%s'不能被实例化")
	#define TXT_OBJECT_s_IS_NOT_INERITABLE		PS_T("类'%s'不能被当作父类,它没有无参构造函数.")
	#define TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP  PS_T("对象没有重载 '[]' 操作符")
	#define TXT_OBJECT_DOESNT_SUPPORT_PRE_OP_s    PS_T("对象没有重载 '%s' 操作符 ")
	#define TXT_OBJECT_DOESNT_SUPPORT_POST_INC_OP PS_T("对象没有重载 '++' 操作符")
	#define TXT_ONLY_ONE_ARGUMENT_IN_CAST       PS_T("类型转化函数中只能有一个参数")

	#define TXT_PARAMETER_ALREADY_DECLARED      PS_T("参数已经被声明")
	#define TXT_PARAMETER_CANT_BE_s             PS_T("参数不能被声明为 '%s'")
	#define TXT_POSSIBLE_LOSS_OF_PRECISION      PS_T("从double类型转化为float,可能丢失数据")

	#define TXT_REF_IS_READ_ONLY                PS_T("引用为只读的")
	#define TXT_REF_IS_TEMP                     PS_T("引用为临时的")

	#define TXT_IF_STATEMENT_ALWAYS_BE_FALSE    PS_T("'if'条件表达式的值总是为假")
	#define TXT_IF_STATEMENT_ALWAYS_BE_TRUE	    PS_T("'if'条件表达式的值总是为真")

	#define TXT_SWITCH_CASE_MUST_BE_CONSTANT    PS_T("case表达式的值必需为常数")
	#define TXT_SWITCH_MUST_BE_INTEGRAL         PS_T("switch表达式的值必需为整数")
	#define TXT_CASE_VALUE_d_ALREADY_USED       PS_T("case值'%d'已经被使用.")

	#define TXT_NAME_s_IS_TOO_LONG				PS_T("'%s'的长度太长,大于255")
	#define TXT_UNEXPECTED_END_OF_FILE          PS_T("不期望的文件尾")
	#define TXT_UNEXPECTED_TOKEN_s              PS_T("不期望的标记 '%s'")
	#define TXT_UNINITIALIZED_GLOBAL_VAR_s      PS_T("使用了没有初始化的全局变量 '%s'.")
	#define TXT_UNREACHABLE_CODE                PS_T("不可到达的代码段")
	#define TXT_UNUSED_SCRIPT_NODE              PS_T("无用的脚本结点")

	#define TXT_VALUE_TOO_LARGE_FOR_TYPE            PS_T("数据的值超出了数据类型的范围")
	#define TXT_SCRIPT_SEFL_DONT_SUPPORT_ARGCHANGED	PS_T("脚本本身并不支持变参")
	#define TXT_ENUM_ALREADY_HAS_FIELD_s			PS_T("枚举类型已经有域名'%s'")

	// Engine message
	#define TXT_INVALID_CONFIGURATION         PS_T("无效的系统配置\n")

	// Message types

	#define TXT_INFO                          PS_T("信息  ")
	#define TXT_ERROR                         PS_T("错误  ")
	#define TXT_WARNING                       PS_T("警告  ")
#else
	// Compiler messages
	#define TXT_s_ALREADY_DECLARED				 PS_T("'%s' already declare")
	#define TXT_ARG_NOT_LVALUE					 PS_T("Argument cannot be assigned")

	#define TXT_CANT_IMPLICITLY_CONVERT_s_TO_s  PS_T("Can't implicitly convert from '%s' to '%s'.")
	#define TXT_CANT_RETURN_VALUE               PS_T("Can't return value when return type is 'void'")
	#define TXT_CHANGE_SIGN_u_d                 PS_T("Implicit conversion changed sign of value, %u -> %d.")
	#define TXT_CHANGE_SIGN_d_u                 PS_T("Implicit conversion changed sign of value, %d -> %u.")
	#define TXT_COMPILING_s                     PS_T("Compiling %s")
	#define TXT_CONST_NOT_PRIMITIVE             PS_T("Only primitives may be declared as const")

	#define TXT_DATA_TYPE_CANT_BE_s             PS_T("Data type can't be '%s'")
	#define TXT_DEFAULT_MUST_BE_LAST            PS_T("The default case must be the last one")

	#define TXT_EXPECTED_s                      PS_T("Expected '%s'")
	#define TXT_EXPECTED_CONSTANT               PS_T("Expected constant")
	#define TXT_ARRAY_SIZE_MUST_BE_INT_CONSTANT PS_T("The size of the array must be interger")
	#define TXT_CONCATE_STRING_TYPE_NOT_MATCH   PS_T("Concatenating string mismatched. ")

	// TODO: Should be TXT_NO_CONVERSION
	#define TXT_EXPECTED_CONSTANT_s_FOUND_s   PS_T("Expected constant of type '%s', found '%s'")
	#define TXT_EXPECTED_DATA_TYPE            PS_T("Expected data type")
	#define TXT_EXPECTED_EXPRESSION_VALUE     PS_T("Expected expression value")
	#define TXT_EXPECTED_IDENTIFIER           PS_T("Expected identifier")
	#define TXT_EXPECTED_ONE_OF               PS_T("Expected one of: ")
	#define TXT_EXPECTED_OPERATOR             PS_T("Expected operator")
	#define TXT_EXPECTED_s_OR_s               PS_T("Expected '%s' or '%s'")
	#define TXT_EXPECTED_POST_OPERATOR        PS_T("Expected post operator")
	#define TXT_EXPECTED_PRE_OPERATOR         PS_T("Expected pre operator")
	#define TXT_EXPECTED_STRING               PS_T("Expected string")
	#define TXT_EXPR_MUST_BE_BOOL             PS_T("Expression must be of boolean type")
	#define TXT_EXPR_MUST_EVAL_TO_CONSTANT    PS_T("Expression must evaluate to a constant")
	#define TXT_MACRO_s_REDEFINITION		  PS_T("The macro '%s' is already defined")
	#define	TXT_MACRO_s_IS_RESREVED			  PS_T("The macro '%s' is reserved, ignore")
	#define TXT_UNEXPECTED_NUMBERELSE		  PS_T("Unexpected '#else'")
	#define TXT_UNEXPECTED_NUMBERENDIF		  PS_T("Unexpected '#endif'")

	#define TXT_INCLUDE_FILE_TOO_MANY_MAX_d	  PS_T("Include file is too many: max level: %d")
	#define TXT_INCLUDE_EXPECT_FILENAME		  PS_T("Expected filename after '#include' ")
	#define TXT_CANT_READ_FILE_s			  PS_T("Can't read file '%s'")

	#define TXT_IFDEF_EXPECTED_IDENTIFIER	  PS_T("Expected identifier after #if[n]def")
	#define TXT_UNDEF_EXPECTED_IDENTIFIER	  PS_T("Expected identifier after #undef")

	#define TXT_FUNCTION_s_ALREADY_EXIST      PS_T("The function '%s' is already existed")

	#define TXT_IDENTIFIER_s_NOT_DATA_TYPE    PS_T("Identifier '%s' is not a data type")
	#define TXT_ILLEGAL_CALL                  PS_T("Invalid call")
	// TODO: Should be TXT_ILLEGAL_OPERATION_ON_s
	#define TXT_ILLEGAL_OPERATION             PS_T("Illegal operation on this datatype")
	#define TXT_ILLEGAL_OPERATION_ON_s        PS_T("Illegal operation on '%s'")

	#define TXT_INVALID_BREAK                 PS_T("Invalid 'break'")
	#define TXT_INVALID_CONTINUE              PS_T("Invalid 'continue'")
	#define TXT_INVALID_TYPE                  PS_T("Invalid type")
	#define TXT_INVALID_OPERATOR			  PS_T("Invalid operator")
	#define TXT_ILLEGAL_VARIABLE_NAME_s       PS_T("Illegal variable name '%s'.")

	#define TXT_MORE_THAN_ONE_MATCHING_OP     PS_T("Found more than one matching operator")
	#define TXT_NO_MATCHING_OP				  PS_T("Can't find matched operator overloading")

	#define TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s PS_T("Multiple matching signatures to '%s'")
	#define TXT_MUST_BE_OBJECT					  PS_T("Only objects have constructors")
	#define TXT_s_MUST_BE_SENT_BY_REF			  PS_T("'%s' must be sent by reference")
	#define TXT_MUST_RETURN_VALUE				  PS_T("Must return a value")

	#define TXT_NAME_CONFILIC_s_OBJ_EXIST		PS_T("Name conflict. '%s' is a already defined")
	#define TXT_NAME_CONFLICT_s_CONSTANT        PS_T("Name conflict. '%s' is a constant.")
	#define TXT_NAME_CONFLICT_s_EXTENDED_TYPE   PS_T("Name conflict. '%s' is an extended data type.")
	#define TXT_NAME_CONFLICT_s_FUNCTION        PS_T("Name conflict. '%s' is a function.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_VAR      PS_T("Name conflict. '%s' is a global variable.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY PS_T("Name conflict. '%s' is a global property.")
	#define TXT_NAME_CONFILIC_s_ENUM_CONSTANT	PS_T("Name conflict. '%s' is a enum constant")
	#define TXT_NAME_CONFLICT_s_OBJ_PROPERTY    PS_T("Name conflict. '%s' is an object property.")
	#define TXT_NAME_CONFLICT_s_OBJ_METHOD      PS_T("Name conflict. '%s' is an object method.")
	#define TXT_NAME_CONFLICT_s_SYSTEM_FUNCTION PS_T("Name conflict. '%s' is a system function.")

	#define TXT_NO_CONVERSION_s_TO_s            PS_T("No conversion from '%s' to '%s' available.")
	#define TXT_NO_CONVERSION_s_TO_MATH_TYPE    PS_T("No conversion from '%s' to math type available.")
	#define TXT_NO_DEFAULT_COPY_OP              PS_T("There is no copy operator for this type available.")
	#define TXT_NO_MATCHING_SIGNATURES_TO_s		  PS_T("No matching signatures to '%s'")
	#define TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s PS_T("No matching operator '%s' that takes the type '%s' found")
	#define TXT_NOT_ALL_PATHS_RETURN            PS_T("Not all paths return a value")
	#define TXT_s_NOT_AVAILABLE_FOR_s           PS_T("'%s' is not available for '%s'")
	#define TXT_s_NOT_DECLARED                  PS_T("'%s' is not declared")
	#define TXT_NOT_EXACT_g_d_g                 PS_T("Implicit conversion of value is not exact, %g -> %d -> %g.")
	#define TXT_NOT_EXACT_d_g_d                 PS_T("Implicit conversion of value is not exact, %d -> %g -> %d.")
	#define TXT_NOT_EXACT_g_u_g                 PS_T("Implicit conversion of value is not exact, %g -> %u -> %g.")
	#define TXT_NOT_EXACT_u_g_u                 PS_T("Implicit conversion of value is not exact, %u -> %g -> %u.")

	#define TXT_CONVERT_s_TO_s_MAY_LOST_DATA    PS_T("Covert '%s' to '%s', may be lost data")

	#define TXT_CANT_CONVERT_s_TO_s			    PS_T("Can't convert '%s' to '%s' ")
	#define TXT_CONVERT_s_TO_s_IS_DANGEROUS		PS_T("Convert '%s' to '%s', is dangerous" )

	#define TXT_REF_TYPE_CANNOT_BE_ARRAY	    PS_T("The data type can't be array when is reference.")
	#define TXT_REF_s_INITEXPR_TYPE_ERROR	    PS_T("The data type of initialized expression is not matched.")
	#define TXT_REF_s_MUST_BE_INITIALIZED       PS_T("Reference '%s' must be initialized.")
	#define TXT_REF_s_CAN_NOT_BE_TEMPOARY	    PS_T("Reference '%s' is temporary.")
	#define TXT_REF_CANNOT_BE_TEMPORARY		    PS_T("Reference is tempoary")

	#define TXT_INIT_ARRAY_SIZE_d_DONOT_MATCH_d	PS_T("Too many initializers(%d != %d)")
	#define TXT_VARIABLE_INITEXPR_TYPE_ERROR    PS_T("Too many initializers")
	#define TXT_CHAR_INIT_EXPR_TOO_FEW		    PS_T("Too few characters in constant.")
	#define TXT_CHAR_INIT_EXPR_TOO_MANY		    PS_T("Too many characters in constant.")

	#define TXT_ARRAY_SIZE_ERROR			    PS_T("Cannot allocate an array of constant size 0")
	#define TXT_ARRAY_DIM_TOO_LARGE			    PS_T("Too many array dimensions.")
	#define TXT_ARRAY_SIZE_TOO_LARGE		    PS_T("The array size is too large.")
	#define TXT_ASSIGN_LEFT_MUSTBE_LVALUE	    PS_T("The left expression cannot be assigned.")
	#define TXT_ARRAY_INDEX_MUSTBE_INTEGER	    PS_T("The index of array must be integer.")
	#define	TXT_REF_s_TYPE_IS_ERROR			    PS_T("The type of Reference '%s' is error!")
	#define TXT_INAVLID_GET_s_POINTER		    PS_T("Invalid operator '&' on '%s' ")
	#define TXT_VAR_ARG_s_DONT_SUPPORT		    PS_T("The type '%s' can't be argument of variational function.")
	#define TXT_VAR_FIRST_ARG_MUST_BE_TEXT	    PS_T("The data type of first argument must be 'LPTSTR' or 'int' for variational function.")
	#define TXT_VAR_FUNC_ARGSIZE_TOO_SMALL	    PS_T("The argument count is too few for variational function.")

	#define TXT_s_NOT_FUNCTION                  PS_T("Function '%s' not found")
	#define TXT_s_NOT_INITIALIZED               PS_T("'%s' is not initialized.")
	#define TXT_s_NOT_MEMBER_OF_s               PS_T("'%s' is not a member of '%s'")
	#define TXT_NOT_VALID_REFERENCE             PS_T("Not a valid reference")

	#define TXT_PROPERTY_s_ALREADY_EXIST	    PS_T("Property '%s' is already existed.")
	#define TXT_PROPERTY_CANNOT_BE_REF			PS_T("Property can't be reference.")
	#define TXT_CONSTRUCTOR_FUNC_NAME_ERROR	    PS_T("The name of constructor must be same as class name.")
	#define TXT_DESTRUCTOR_FUNC_NAME_ERROR	    PS_T("The name of destructor must be same as class name.")
	#define TXT_DESTRUCTOR_ALREADY_EXIST	    PS_T("The desctructor is already existed.")
	#define TXT_DESTRUCTOR_DONT_HAS_ARGS	    PS_T("The desctructor doesnot have arguments.")
	#define TXT_METHOD_s_ALREADY_EXIST		    PS_T("The method '%s' is already existed.")
	#define TXT_CONSTRUCTOR_s_ALREADY_EXIST	    PS_T("The constructor '%s' is already existed.")
	#define TXT_OPOVERLOAD_s_ALREADY_EXIST      PS_T("The overloaded operator '%s' is already existed. ")
	#define TXT_OPOVERLOAD_s_ONLY_HAS_TWOPARAM	PS_T("The argument count is two for overloaded operator '%s'")
	#define TXT_OPOVERLOAD_s_HAS_TOOMANY_PARAM	PS_T("The argument count is too many for overloaded operator '%s'" )
	#define TXT_OP_s_NOT_ALLOW_OVERLOAD			PS_T("The operator '%s 'can't be overloaded")

	#define TXT_METHODNAME_s_SAMEAS_CLASSNAME   PS_T("The name of method '%s' is same as class name")
	#define TXT_INVALID_THIS_POINTER		    PS_T("Invalid this pointer")
	#define	TXT_OBJECT_s_CANNOT_BE_INSTANCE     PS_T("The object '%s' can't be instanced.")
	#define TXT_OBJECT_s_IS_NOT_INERITABLE		PS_T("The object'%s'is not inheriable. it doesnot has default constrouctor.")
	#define TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP  PS_T("The object doesn't support the indexing operator.")
	#define TXT_OBJECT_DOESNT_SUPPORT_PRE_OP_s    PS_T("The object doesn't support the '%s' operator.")
	#define TXT_OBJECT_DOESNT_SUPPORT_POST_INC_OP PS_T("The object doesn't support the post increment operator.")
	#define TXT_ONLY_ONE_ARGUMENT_IN_CAST       PS_T("The type cast only has one argument.")

	#define TXT_PARAMETER_ALREADY_DECLARED      PS_T("Parameter already declared")
	#define TXT_PARAMETER_CANT_BE_s             PS_T("Parameter type can't be '%s'")
	#define TXT_POSSIBLE_LOSS_OF_PRECISION      PS_T("Conversion from double to float, possible loss of precision")

	#define TXT_REF_IS_READ_ONLY                PS_T("Reference is read-only")
	#define TXT_REF_IS_TEMP                     PS_T("Reference is temporary")

	#define TXT_IF_STATEMENT_ALWAYS_BE_FALSE    PS_T("The 'if' condition expression is always be false.")
	#define TXT_IF_STATEMENT_ALWAYS_BE_TRUE	    PS_T("The 'if' condition expression is always be true.")

	#define TXT_SWITCH_CASE_MUST_BE_CONSTANT    PS_T("case expression not constant.")
	#define TXT_SWITCH_MUST_BE_INTEGRAL         PS_T("switch expression not integer.")
	#define TXT_CASE_VALUE_d_ALREADY_USED       PS_T("case value '%d' already used.")

	#define TXT_NAME_s_IS_TOO_LONG				PS_T("The length of '%s' is too long,greater than 255")
	#define TXT_UNEXPECTED_END_OF_FILE          PS_T("Unexpected end of file.")
	#define TXT_UNEXPECTED_TOKEN_s              PS_T("Unexpected token '%s'.")
	#define TXT_UNINITIALIZED_GLOBAL_VAR_s      PS_T("Use of uninitialized global variable '%s'.")
	#define TXT_UNREACHABLE_CODE                PS_T("Unreachable code.")
	#define TXT_UNUSED_SCRIPT_NODE              PS_T("Unused script node.")

	#define TXT_VALUE_TOO_LARGE_FOR_TYPE            PS_T("Value is too large for data type.")
	#define TXT_SCRIPT_SEFL_DONT_SUPPORT_ARGCHANGED	PS_T("Script don't support variational funciton.")
	#define TXT_ENUM_ALREADY_HAS_FIELD_s			PS_T("The enum type already has field name '%s'." )
	// Engine message

	#define TXT_INVALID_CONFIGURATION         PS_T("Invalid configuration\n")

	// Message types

	#define TXT_INFO                          PS_T("Info   ")
	#define TXT_ERROR                         PS_T("Error  ")
	#define TXT_WARNING                       PS_T("Warning")
#endif // LANGUAGE_CHINESE

	// Internal names
	#define TXT_EXECUTESTRING                   PS_T("ExecuteString")
	#define TXT_PROPERTY						PS_T("Property")
	#define TXT_SYSTEM_FUNCTION                 PS_T("System function")
	#define TXT_VARIABLE_DECL                   PS_T("Variable declaration")

	// Exceptions
	#define TXT_SYSTEM_FUNCTION_NOTFIND		    PS_T("The system funciton can't be find.")
	#define TXT_SYSTEM_EXECEPTION				PS_T("The system exception. ")

	#define TXT_STACK_OVERFLOW                  PS_T("Stack overflow")
	#define TXT_NULL_POINTER_ACCESS             PS_T("Null pointer access")
	#define TXT_MISALIGNED_STACK_POINTER        PS_T("Misaligned stack pointer")
	#define TXT_DIVIDE_BY_ZERO                  PS_T("Divide by zero")
	#define TXT_UNRECOGNIZED_BYTE_CODE          PS_T("Unrecognized byte code")
	#define TXT_INVALID_CALLING_CONVENTION      PS_T("Invalid calling convention")
#endif // __PS_TEXT_H__
