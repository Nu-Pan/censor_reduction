
#pragma once

namespace utl
{
	namespace mcr
	{
		namespace impl
		{
			/*! 指定されたossの末尾にファイル名と行数を付加してruntime_errorとしてthrow
			 * 渡されたossに操作が加えられるため注意
			 * @param [in, out] oss	エラーメッセージが格納されているout string stream.
			 * @param [in] file	ファイル名.普通は__FILE__を渡す.
			 * @param [in] line 行数.普通は__LINE__を渡す.
			 */
			inline void ThrowOss(std::ostringstream& oss, const char* const file, int line)
			{
				oss << "FILE : " << file << "\n";
				oss << "LINE : " << line << "\n";
				throw std::runtime_error(oss.str());
			}
		};
	};
};

/*! ossにファイル名と行数を付加してruntime_errorとしてthrow
 * ossに操作が加えられるため注意
 */
#define THROW_OSS(oss) utl::mcr::impl::ThrowOss(oss, __FILE__, __LINE__)

namespace utl
{
	namespace mcr
	{
		namespace impl
		{
			/*! UTL_ASSERT()の実装関数(ダミー)
			* 最適化による消去が期待される
			*/
			inline void UtlAssert()
			{
			}

			/*! UTL_ASSERT()の実装関数
			* @param [in] expression_bool	評価式の値
			* @param [in] expression_str	文字列化された評価式
			* @param [in] file				呼び出されたファイル名
			* @param [in] line				呼び出された行数
			*/
			inline void UtlAssert(bool expression_bool, const char * const expression_str, const char* const file, int line)
			{
				//式がtrueならOK
				if(expression_bool)
				{
					return;
				}
				//式がfalseなら例外を投げる
				std::ostringstream oss;
				oss << "UTL_ASSERT(exp) has failed.\n";
				oss << "Expression : " << expression_str << "\n";
				ThrowOss(oss, file, line);
			}

			/** UTL_PRMCHKの実装関数
			* @param [in] expression_bool	評価式の値
			* @param [in] expression_str	文字列化された評価式
			* @param [in] file				呼び出されたファイル名
			* @param [in] line				呼び出された行数
			*/
			inline void ParameterCheck(bool expression_bool, const char * const expression_str, const char* const file, int line)
			{
				//式がtrueならOK
				if (expression_bool) 
				{
					return;
				}
				//式がfalseなら例外を投げる
				std::ostringstream oss;
				oss << "UTL_PRMCHK(exp) has failed.\n";
				oss << "Expression : " << expression_str << "\n";
				ThrowOss(oss, file, line);
			}
		};
	};
};

#ifdef FORCE_ENABLE_UTL_ASSERT
#define UTL_ASSERT(expression) utl::mcr::impl::UtlAssert(expression, #expression, __FILE__, __LINE__)
#else
#ifdef DEBUG
#define UTL_ASSERT(expression) utl::mcr::impl::UtlAssert(expression, #expression, __FILE__, __LINE__)
#else
#define UTL_ASSERT(expression) utl::mcr::impl::UtlAssert()
#endif
#endif

#define UTL_PRMCHK(expression) utl::mcr::impl::ParameterCheck(expression, #expression, __FILE__, __LINE__)
