#ifndef __PS_TEXT_H__
#define __PS_TEXT_H__

#ifdef LANGUAGE_CHINESE
	// Compiler messages
	#define TXT_s_ALREADY_DECLARED				 PS_T("'%s' �Ѿ���������.")
	#define TXT_ARG_NOT_LVALUE					 PS_T("��ֵ����������ֵ")

	#define TXT_CANT_IMPLICITLY_CONVERT_s_TO_s  PS_T("���ܹ������Ľ� '%s' ת��Ϊ '%s'.")
	#define TXT_CANT_RETURN_VALUE               PS_T("��������Ϊ'void'ʱ���ܷ���ֵ")
	#define TXT_CHANGE_SIGN_u_d                 PS_T("����ת���ı���ֵ�ķ���, %u -> %d.")
	#define TXT_CHANGE_SIGN_d_u                 PS_T("����ת���ı���ֵ�ķ���, %d -> %u.")
	#define TXT_COMPILING_s                     PS_T("���� %s")
	#define TXT_CONST_NOT_PRIMITIVE             PS_T("ֻ�л��������ܱ�����Ϊ'const'")

	#define TXT_DATA_TYPE_CANT_BE_s             PS_T("�������Ͳ���Ϊ '%s'")
	#define TXT_DEFAULT_MUST_BE_LAST            PS_T("'default'����������'case'�ĺ���")

	#define TXT_EXPECTED_s                      PS_T("�������� '%s'")
	#define TXT_EXPECTED_CONSTANT               PS_T("�������ֳ���")
	#define TXT_ARRAY_SIZE_MUST_BE_INT_CONSTANT PS_T("�����С���������ͳ���")
	#define TXT_CONCATE_STRING_TYPE_NOT_MATCH   PS_T("�ַ�������ʱ���Ͳ�ƥ��")
	// TODO: Should be TXT_NO_CONVERSION
	#define TXT_EXPECTED_CONSTANT_s_FOUND_s   PS_T("�������ֳ������� '%s', ȴ������ '%s'")
	#define TXT_EXPECTED_DATA_TYPE            PS_T("����������������")
	#define TXT_EXPECTED_EXPRESSION_VALUE     PS_T("�������ֱ��ʽ")
	#define TXT_EXPECTED_IDENTIFIER           PS_T("�������ֱ�ʶ��")
	#define TXT_EXPECTED_ONE_OF               PS_T("�����������е�һ��: ")
	#define TXT_EXPECTED_OPERATOR             PS_T("�������ֲ�����")
	#define TXT_EXPECTED_s_OR_s               PS_T("�������� %s' �� '%s'")
	#define TXT_EXPECTED_POST_OPERATOR        PS_T("�������ֺ��ò�����")
	#define TXT_EXPECTED_PRE_OPERATOR         PS_T("��������ǰ�ò�����")
	#define TXT_EXPECTED_STRING               PS_T("���������ַ���")
	#define TXT_EXPR_MUST_BE_BOOL             PS_T("���ʽ����Ϊ'bool'����")
	#define TXT_EXPR_MUST_EVAL_TO_CONSTANT    PS_T("���ʽ����Ϊһ������ֵ")
	#define TXT_MACRO_s_REDEFINITION		  PS_T("�� '%s' �ض���")
	#define	TXT_MACRO_s_IS_RESREVED			  PS_T("�� '%s'������������, ����")
	#define TXT_UNEXPECTED_NUMBERELSE		  PS_T("���������ֵ� #else")
	#define TXT_UNEXPECTED_NUMBERENDIF		  PS_T("���������ֵ� #endif")

	#define TXT_INCLUDE_FILE_TOO_MANY_MAX_d	  PS_T("�������ļ�̫��: ������Ϊ: %d")
	#define TXT_INCLUDE_EXPECT_FILENAME		  PS_T("��'include'����������һ���ļ���, �������˻���")
	#define TXT_CANT_READ_FILE_s			  PS_T("���ܶ�ȡ�ļ�'%s'")

	#define TXT_IFDEF_EXPECTED_IDENTIFIER	  PS_T("#if[n]def ��������һ����ʶ��, �������˻��з�")
	#define TXT_UNDEF_EXPECTED_IDENTIFIER	  PS_T("#undef ��������һ����ʶ��, �������˻��з�")

	#define TXT_FUNCTION_s_ALREADY_EXIST      PS_T("�Ѿ���һ��ͬ���ƺ����͵ĺ���'%s'����")

	#define TXT_IDENTIFIER_s_NOT_DATA_TYPE    PS_T("��ʶ�� '%s' ������һ����������")
	#define TXT_ILLEGAL_CALL                  PS_T("��Ч�ĺ�������")
	// TODO: Should be TXT_ILLEGAL_OPERATION_ON_s
	#define TXT_ILLEGAL_OPERATION             PS_T("�Ƿ��Ĳ�����")
	#define TXT_ILLEGAL_OPERATION_ON_s        PS_T("�� '%s' �Ĳ������Ƿ�")

	#define TXT_INVALID_BREAK                 PS_T("��Ч�� 'break'")
	#define TXT_INVALID_CONTINUE              PS_T("��Ч�� 'continue'")
	#define TXT_INVALID_TYPE                  PS_T("��Ч������")
	#define TXT_INVALID_OPERATOR			  PS_T("��Ч�Ĳ���")
	#define TXT_ILLEGAL_VARIABLE_NAME_s       PS_T("���Ϸ��ĵı����� '%s',�Ѿ���һ����ռ�����������.")

	#define TXT_MORE_THAN_ONE_MATCHING_OP     PS_T("�ҵ��˶��ƥ������ز�����")
	#define TXT_NO_MATCHING_OP				  PS_T("û���ҵ����ز���")

	#define TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s PS_T("�ҵ���'%s' �ж����ͬ��ƥ��ĺ���")
	#define TXT_MUST_BE_OBJECT					  PS_T("ֻ���������й��캯��")
	#define TXT_s_MUST_BE_SENT_BY_REF			  PS_T("'%s' �������������ʹ���")
	#define TXT_MUST_RETURN_VALUE				  PS_T("�������践��ֵ")

	#define TXT_NAME_CONFILIC_s_OBJ_EXIST		PS_T("���ֳ�ͻ. ����Ϊ'%s'�����Ѿ�����")
	#define TXT_NAME_CONFLICT_s_CONSTANT        PS_T("���ֳ�ͻ. '%s' ��һ������.")
	#define TXT_NAME_CONFLICT_s_EXTENDED_TYPE   PS_T("���ֳ�ͻ. '%s' ��һ����չ������������(������).")
	#define TXT_NAME_CONFLICT_s_FUNCTION        PS_T("���ֳ�ͻ. '%s' ��һ������.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_VAR      PS_T("���ֳ�ͻ. '%s' ��һ��ȫ�ֱ���.")
	#define TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY PS_T("���ֳ�ͻ. '%s' ��һ��ȫ������.")
	#define TXT_NAME_CONFILIC_s_ENUM_CONSTANT	PS_T("���ֳ�ͻ. '%s' ��һ��ö�ٳ���.")
	#define TXT_NAME_CONFLICT_s_OBJ_PROPERTY    PS_T("���ֳ�ͻ. '%s' ��һ����������.")
	#define TXT_NAME_CONFLICT_s_OBJ_METHOD      PS_T("���ֳ�ͻ. '%s' ��һ�����󷽷�.")
	#define TXT_NAME_CONFLICT_s_SYSTEM_FUNCTION PS_T("���ֳ�ͻ. '%s' ��һ��ϵͳ����.")

	#define TXT_NO_CONVERSION_s_TO_s            PS_T("û�д� '%s' �� '%s' ��ת��.")
	#define TXT_NO_CONVERSION_s_TO_MATH_TYPE    PS_T("û�д� '%s' �� ��ֵ���͵�ת��.")
	#define TXT_NO_DEFAULT_COPY_OP              PS_T("�����������û��һ����Ч�Ŀ�������.")
	#define TXT_NO_MATCHING_SIGNATURES_TO_s     PS_T("û���ҵ��� '%s' ƥ��ĺ���")
	#define TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s PS_T("û���ҵ�ƥ������ز�����'%s'����, ���� '%s' ")
	#define TXT_NOT_ALL_PATHS_RETURN            PS_T("����������·�����з���ֵ")
	#define TXT_s_NOT_AVAILABLE_FOR_s           PS_T("'%s' �� '%s'��Ч")
	#define TXT_s_NOT_DECLARED                  PS_T("'%s' û������")
	#define TXT_NOT_EXACT_g_d_g                 PS_T("��ֵ������ת��������ȷ, %g -> %d -> %g.")
	#define TXT_NOT_EXACT_d_g_d                 PS_T("��ֵ������ת��������ȷ, %d -> %g -> %d.")
	#define TXT_NOT_EXACT_g_u_g                 PS_T("��ֵ������ת��������ȷ, %g -> %u -> %g.")
	#define TXT_NOT_EXACT_u_g_u                 PS_T("��ֵ������ת��������ȷ, %u -> %g -> %u.")

	#define TXT_CONVERT_s_TO_s_MAY_LOST_DATA    PS_T("�� '%s' ת���� '%s', ���ܶ�ʧ����")

	#define TXT_CANT_CONVERT_s_TO_s			    PS_T("���ܴ� '%s' ת���� '%s' ")
	#define TXT_CONVERT_s_TO_s_IS_DANGEROUS		PS_T("�� '%s' ת���� '%s', ���ܺ�Σ��" )

	#define TXT_REF_TYPE_CANNOT_BE_ARRAY	    PS_T("�������Ͳ���Ϊһ������")
	#define TXT_REF_s_INITEXPR_TYPE_ERROR	    PS_T("���ñ�����ʼ�����ʽ�����Ͳ���ȷ!")
	#define TXT_REF_s_MUST_BE_INITIALIZED       PS_T("���� '%s' �����ʼ��")
	#define TXT_REF_s_CAN_NOT_BE_TEMPOARY	    PS_T("���� '%s' ����Ϊ��ʱ����")
	#define TXT_REF_CANNOT_BE_TEMPORARY		    PS_T("���ò���Ϊ��ʱ��")

	#define TXT_INIT_ARRAY_SIZE_d_DONOT_MATCH_d	PS_T("��ʼ�������С(%d)�������Ĵ�С(%d)��ƥ��")
	#define TXT_VARIABLE_INITEXPR_TYPE_ERROR    PS_T("������ʼ�����ʽ�Ĵ�С����ȷ")
	#define TXT_CHAR_INIT_EXPR_TOO_FEW		    PS_T("�ַ��ͳ�����ʼ�����ַ�����̫��")
	#define TXT_CHAR_INIT_EXPR_TOO_MANY		    PS_T("�ַ��ͳ�����ʼ�����ַ�����̫��")

	#define TXT_ARRAY_SIZE_ERROR			    PS_T("����Ĵ�С����Ϊ����1���޷������ͳ���!")
	#define TXT_ARRAY_DIM_TOO_LARGE			    PS_T("�����ά��̫��")
	#define TXT_ARRAY_SIZE_TOO_LARGE		    PS_T("����Ĵ�С̫��")
	#define TXT_ASSIGN_LEFT_MUSTBE_LVALUE	    PS_T("��ֵ���ʽ����߱�������ֵ"	)
	#define TXT_ARRAY_INDEX_MUSTBE_INTEGER	    PS_T("������±�һ��ҪΪ����")
	#define	TXT_REF_s_TYPE_IS_ERROR			    PS_T("����'%s'�����ͳ���!")
	#define TXT_INAVLID_GET_s_POINTER		    PS_T("������'%s'��ȡ��ַ������Ч")
	#define TXT_VAR_ARG_s_DONT_SUPPORT		    PS_T("����'%s'��֧����Ϊ��κ����Ĳ���")
	#define TXT_VAR_FIRST_ARG_MUST_BE_TEXT	    PS_T("��κ����ĵ�һ����������Ϊ���ֳ����������ͳ���.")
	#define TXT_VAR_FUNC_ARGSIZE_TOO_SMALL	    PS_T("��κ����Ĳ�������̫��(����Ϊһ��)")

	#define TXT_s_NOT_FUNCTION                  PS_T("û���ҵ����� '%s'")
	#define TXT_s_NOT_INITIALIZED               PS_T("'%s' û��ʼ��.")
	#define TXT_s_NOT_MEMBER_OF_s               PS_T("'%s' ���� '%s' �ĳ�Ա")
	#define TXT_NOT_VALID_REFERENCE             PS_T("��Ч������")

	#define TXT_PROPERTY_s_ALREADY_EXIST	    PS_T("����'%s'�Ѿ�����")
	#define TXT_PROPERTY_CANNOT_BE_REF			PS_T("���Ա���Բ���Ϊ��������")
	#define TXT_CONSTRUCTOR_FUNC_NAME_ERROR	    PS_T("���캯�������Ʊ����������ͬ")
	#define TXT_DESTRUCTOR_FUNC_NAME_ERROR	    PS_T("�������������Ʊ����������ͬ")
	#define TXT_DESTRUCTOR_ALREADY_EXIST	    PS_T("���������Ѿ�����")
	#define TXT_DESTRUCTOR_DONT_HAS_ARGS	    PS_T("�������������в���")
	#define TXT_METHOD_s_ALREADY_EXIST		    PS_T("�෽��'%s'�Ѿ�����")
	#define TXT_CONSTRUCTOR_s_ALREADY_EXIST	    PS_T("���캯��'%s'�Ѿ�����")
	#define TXT_OPOVERLOAD_s_ALREADY_EXIST      PS_T("�Բ�����'%s'�������Ѿ�����")
	#define TXT_OPOVERLOAD_s_ONLY_HAS_TWOPARAM	PS_T("���ز�����'%s'�Ĳ���ֻ������")
	#define TXT_OPOVERLOAD_s_HAS_TOOMANY_PARAM	PS_T("���ز�����'%s'�Ĳ���̫��")
	#define TXT_OP_s_NOT_ALLOW_OVERLOAD			PS_T("������'%s'����������")

	#define TXT_METHODNAME_s_SAMEAS_CLASSNAME   PS_T("����'%s'����������")
	#define TXT_INVALID_THIS_POINTER		    PS_T("��Ч��thisָ��")
	#define	TXT_OBJECT_s_CANNOT_BE_INSTANCE     PS_T("��'%s'���ܱ�ʵ����")
	#define TXT_OBJECT_s_IS_NOT_INERITABLE		PS_T("��'%s'���ܱ���������,��û���޲ι��캯��.")
	#define TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP  PS_T("����û������ '[]' ������")
	#define TXT_OBJECT_DOESNT_SUPPORT_PRE_OP_s    PS_T("����û������ '%s' ������ ")
	#define TXT_OBJECT_DOESNT_SUPPORT_POST_INC_OP PS_T("����û������ '++' ������")
	#define TXT_ONLY_ONE_ARGUMENT_IN_CAST       PS_T("����ת��������ֻ����һ������")

	#define TXT_PARAMETER_ALREADY_DECLARED      PS_T("�����Ѿ�������")
	#define TXT_PARAMETER_CANT_BE_s             PS_T("�������ܱ�����Ϊ '%s'")
	#define TXT_POSSIBLE_LOSS_OF_PRECISION      PS_T("��double����ת��Ϊfloat,���ܶ�ʧ����")

	#define TXT_REF_IS_READ_ONLY                PS_T("����Ϊֻ����")
	#define TXT_REF_IS_TEMP                     PS_T("����Ϊ��ʱ��")

	#define TXT_IF_STATEMENT_ALWAYS_BE_FALSE    PS_T("'if'�������ʽ��ֵ����Ϊ��")
	#define TXT_IF_STATEMENT_ALWAYS_BE_TRUE	    PS_T("'if'�������ʽ��ֵ����Ϊ��")

	#define TXT_SWITCH_CASE_MUST_BE_CONSTANT    PS_T("case���ʽ��ֵ����Ϊ����")
	#define TXT_SWITCH_MUST_BE_INTEGRAL         PS_T("switch���ʽ��ֵ����Ϊ����")
	#define TXT_CASE_VALUE_d_ALREADY_USED       PS_T("caseֵ'%d'�Ѿ���ʹ��.")

	#define TXT_NAME_s_IS_TOO_LONG				PS_T("'%s'�ĳ���̫��,����255")
	#define TXT_UNEXPECTED_END_OF_FILE          PS_T("���������ļ�β")
	#define TXT_UNEXPECTED_TOKEN_s              PS_T("�������ı�� '%s'")
	#define TXT_UNINITIALIZED_GLOBAL_VAR_s      PS_T("ʹ����û�г�ʼ����ȫ�ֱ��� '%s'.")
	#define TXT_UNREACHABLE_CODE                PS_T("���ɵ���Ĵ����")
	#define TXT_UNUSED_SCRIPT_NODE              PS_T("���õĽű����")

	#define TXT_VALUE_TOO_LARGE_FOR_TYPE            PS_T("���ݵ�ֵ�������������͵ķ�Χ")
	#define TXT_SCRIPT_SEFL_DONT_SUPPORT_ARGCHANGED	PS_T("�ű�������֧�ֱ��")
	#define TXT_ENUM_ALREADY_HAS_FIELD_s			PS_T("ö�������Ѿ�������'%s'")

	// Engine message
	#define TXT_INVALID_CONFIGURATION         PS_T("��Ч��ϵͳ����\n")

	// Message types

	#define TXT_INFO                          PS_T("��Ϣ  ")
	#define TXT_ERROR                         PS_T("����  ")
	#define TXT_WARNING                       PS_T("����  ")
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
