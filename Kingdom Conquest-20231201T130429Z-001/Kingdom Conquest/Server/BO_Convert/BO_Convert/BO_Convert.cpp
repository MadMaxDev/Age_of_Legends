#include "stdafx.h"
#include "PAFConvert.h"
#include "PMFConvert.h"
#include "PSFConvert.h"

static void EnumFilesInDir( std::vector< std::wstring >& _Files , const std::wstring& _Path , const std::wstring& _Ext )
{
	WIN32_FIND_DATAW fd;
	HANDLE fHandle = ::FindFirstFileW( ( _Path + L"\\*.*" ).c_str() , &fd ); 
	if( fHandle != INVALID_HANDLE_VALUE )
	{	
		do
		{
			if( ! ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
			{
				std::wstring file_name = fd.cFileName;
				int s = int( file_name.size() );
				if( s > 4 )
				{
					const wchar_t* pExt = file_name.c_str();
					pExt += s - 3;	
					if( _wcsicmp( pExt , _Ext.c_str() ) == 0 )
					{	
						_Files.push_back( file_name );
					}
				}
			}
		}
		while( FindNextFileW( fHandle , &fd ) ) ;
		::FindClose( fHandle );
	}
}

template< typename T > inline 
static void Convert( const std::wstring& _SrcPath , const std::wstring& _DstPath )
{
	T convert;
	BinStream load_stream( _SrcPath.c_str() , L"rb" );
	BinStream save_stream( _DstPath.c_str() , L"wb" );
	if( ! load_stream.IsOK() || ! convert.Load( load_stream ) )
	{
		wprintf( L"open file %s error!\r\n" , _SrcPath.c_str() );
	}
	if( ! save_stream.IsOK() )
	{
		wprintf( L"open file %s error!\r\n" , _DstPath.c_str() );
	}
	convert.Save( save_stream );
	wprintf( L"convert file %s succeed.\r\n" , _DstPath.c_str() );
}

std::wstring GetExePath()
{
	wchar_t szPath[ MAX_PATH ];
	::ZeroMemory( szPath , sizeof( szPath ) );
	::GetModuleFileNameW( NULL , szPath , MAX_PATH );
	for( int i = MAX_PATH - 1 ; i > 0 ; --i )
	{
		if( szPath[ i ] == L'\\' || szPath[ i ] == L'/' )
		{
			szPath[ i ] = L'\0';
			break;
		}
	}
	return szPath;
}

int main(int argc, char* argv[])
{
	::setlocale( LC_ALL , "chs" );

	std::wstring	exe_path = GetExePath();

	std::wstring xx_animation_path	= exe_path + L"\\XX\\Animation";
	std::wstring xx_mesh_path		= exe_path + L"\\XX\\Mesh";
	std::wstring xx_texture_path	= exe_path + L"\\XX\\Texture";
	std::wstring xx_config_path		= exe_path + L"\\XX\\Config";
	std::wstring xx_skeleton_path	= exe_path + L"\\XX\\Skeleton";
	
	std::wstring bo_root_path		= exe_path + L"\\BO";
	std::wstring bo_animation_path	= exe_path + L"\\BO\\Animation";
	std::wstring bo_mesh_path		= exe_path + L"\\BO\\Mesh";
	std::wstring bo_texture_path	= exe_path + L"\\BO\\Texture";
	std::wstring bo_config_path		= exe_path + L"\\BO\\Config";
	std::wstring bo_skeleton_path	= exe_path + L"\\BO\\Skeleton";
	
	::CreateDirectoryW( bo_root_path.c_str() , NULL );
	::CreateDirectoryW( bo_animation_path.c_str() , NULL );
	::CreateDirectoryW( bo_mesh_path.c_str() , NULL );
	::CreateDirectoryW( bo_config_path.c_str() , NULL );
	::CreateDirectoryW( bo_skeleton_path.c_str() , NULL );
	::CreateDirectoryW( bo_texture_path.c_str() , NULL );

	{
		std::vector< std::wstring > Files;
		EnumFilesInDir( Files , xx_skeleton_path , L"PSF" );
		for( size_t i = 0 ; i < Files.size() ; ++i )
		{
			Convert< PSFConvert >( xx_skeleton_path + L"\\" + Files[ i ] , bo_skeleton_path + L"\\"+ Files[ i ] );
		}
	}

	{
		std::vector< std::wstring > Files;
		EnumFilesInDir( Files , xx_animation_path , L"PAF" );
		for( size_t i = 0 ; i < Files.size() ; ++i )
		{
			Convert< PAFConvert >( xx_animation_path + L"\\"+ Files[ i ] , bo_animation_path + L"\\"+ Files[ i ] );
		}
	}

	{
		std::vector< std::wstring > Files;
		EnumFilesInDir( Files , xx_mesh_path , L"PMF" );
		for( size_t i = 0 ; i < Files.size() ; ++i )
		{
			Convert< PMFConvert >( xx_mesh_path + L"\\"+ Files[ i ] , bo_mesh_path + L"\\"+ Files[ i ] );
		}
	}

	wprintf( L"any key exit" );
	getchar();
	return 0;
}

