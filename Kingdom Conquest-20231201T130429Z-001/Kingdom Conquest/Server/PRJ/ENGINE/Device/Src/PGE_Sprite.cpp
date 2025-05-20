#include "../include/PGE_Sprite.h"
#include <assert.h>
#include <vector>
using std::vector;

class PGESprite: public IPGESprite
{
	PGE_NOCOPY_ASSIGN(PGESprite)

	struct _item
	{
		ITexture*		pTexture;
		bool			UsingAlpha;
		bool			UsingColor;

		float tu0;
		float tv0;
		float tu1;
		float tv1;
		float Height;
		float Width;

		D3DXVECTOR3		Position;
		D3DXVECTOR2		Scaling;
		DWORD			Color;
	};


	struct VERTEX
	{
		void set( float _x, float _y, float _z, float _u, float _v)
		{
			x = _x;
			y = _y;
			z = _z;
			w = 1.0f;
			u = _u;
			v = _v;
		}
		float x, y, z, w;
		float u, v;
	};

private:

	vector< _item >	m_Items;

	LONG			m_RefCount;

	IGraphicDevice* m_pDevice;

	IVertexBuffer*  m_pVB;

public:

	PGESprite();
	
	~PGESprite();

	BOOL Create( IGraphicDevice* pDevice );

public:

	virtual ULONG AddRef();
	
	virtual ULONG Release();

	virtual HRESULT Draw(
		  ITexture* pTexture
		, bool _UsingAlpha
		, bool _UsingColor
		, const RECT* pSrcRect
		, const D3DXVECTOR3* pPosition
		, const D3DXVECTOR2* pScaling
		, DWORD Color);
	
	virtual HRESULT Render();
};

//////////////////////////////////////////////////////////////////////////

PGESprite::PGESprite() : m_pDevice( NULL ) , m_pVB( NULL ) , m_RefCount( 0UL )
{

}

PGESprite::~PGESprite()
{
	SAFE_RELEASE( m_pVB );
}

HRESULT PGESprite::Draw(
	  ITexture* pTexture
	, bool _UsingAlpha
	, bool _UsingColor
	, const RECT* pSrcRect
	, const D3DXVECTOR3* pPosition
	, const D3DXVECTOR2* pScaling
	, DWORD Color)
{
	if( pTexture == NULL )
	{
		return -1;
	}
	_item item;

	item.UsingAlpha = _UsingAlpha;

	item.UsingColor = _UsingColor;

	item.pTexture = pTexture;

	D3DSURFACE_DESC desc;

	pTexture->GetLevelDesc(0, &desc);

	float w = (float)desc.Width;

	float h = (float)desc.Height;

	item.Width = w;

	item.Height = h;

	if( pSrcRect != NULL )
	{
		item.tu0 = (float)(pSrcRect->left)	/	w;
		item.tu1 = (float)(pSrcRect->right)	/	w;
		item.tv0 = (float)(pSrcRect->top)	/	h;
		item.tv1 = (float)(pSrcRect->bottom)/	h;
	}
	else
	{
		item.tu0 = 0.0f;
		item.tv0 = 0.0f;
		item.tu1 = 1.0f;
		item.tv1 = 1.0f;
	}

	if( pPosition != NULL )
	{
		item.Position = *pPosition;
	}
	else
	{
		item.Position = D3DXVECTOR3(0, 0, 0);
	}

	if( pScaling != NULL )
	{
		item.Scaling = *pScaling;
	}
	else
	{
		item.Scaling = D3DXVECTOR2(1, 1);
	}

	item.Color = Color;

	m_Items.push_back( item );

	return S_OK;
}


BOOL PGESprite::Create( IGraphicDevice* _pDevice )
{
	m_pDevice = _pDevice;

	m_pVB = m_pDevice->CreateVertexBuffer( 
		4 * sizeof(VERTEX)
		, D3DFVF_XYZRHW|D3DFVF_TEX1
		, 0
		, D3DPOOL_DEFAULT );

	m_RefCount = 1;

	return TRUE;
}


ULONG PGESprite::AddRef()
{
	return (ULONG)InterlockedIncrement((LONG*)&m_RefCount);
}

ULONG PGESprite::Release()
{
	if (InterlockedDecrement((LONG*)&m_RefCount) == 0L )
	{
		delete this;

		return 0;
	}
	else
	{
		return m_RefCount;
	}
}

HRESULT PGESprite::Render()
{
	m_pDevice->SetDefaultRenderState();
	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE);
	m_pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pDevice->SetRenderState( D3DRS_ALPHAREF, 0x0);
	m_pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );
	m_pDevice->SetVertexShader( NULL );
	m_pDevice->SetPixelShader( NULL );
//	m_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
//	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
	m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	for( size_t i = 0 ; i < m_Items.size() ; ++i )
	{
		_item& item = m_Items[ i ];
	
		VERTEX* pVerts = (VERTEX*)m_pVB->Lock(0, 0, D3DLOCK_DISCARD);
		
		float dx = item.Width*item.Scaling.x;

		float dy = item.Height*item.Scaling.y;

		pVerts[0].set(	item.Position.x + 0 
			,			item.Position.y + dy
			,			item.Position.z
			,			item.tu0
			,			item.tv1 );

		pVerts[1].set(	item.Position.x + 0
			,			item.Position.y + 0
			,			item.Position.z
			,			item.tu0
			,			item.tv0 );

		pVerts[2].set(	item.Position.x + dx 
			,			item.Position.y + dy
			,			item.Position.z
			,			item.tu1
			,			item.tv1 );

		pVerts[3].set(	item.Position.x + dx
			,			item.Position.y + 0
			,			item.Position.z
			,			item.tu1
			,			item.tv0 );
		
		m_pVB->Unlock();

		m_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR , item.Color);

		DWORD ColorOP = item.UsingColor ? D3DTOP_MODULATE : D3DTOP_SELECTARG2;
		DWORD AlphaOP = item.UsingAlpha ? D3DTOP_MODULATE : D3DTOP_SELECTARG2;

		m_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP , ColorOP );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP , AlphaOP );
		m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(VERTEX) );
		m_pDevice->SetTexture( 0, item.pTexture );
		m_pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}

	m_Items.clear();

	return S_OK;
}


//--------------------------------------------------------------------
// Name : PGECreateSprite
// Desc : ´´½¨2D¾«Áé
//--------------------------------------------------------------------
DEVICE_EXPORT IPGESprite* WINAPI PGECreateSprite(IGraphicDevice* pDevice)
{
	PGESprite* pSprite = new PGESprite();
	if (pSprite->Create(pDevice))
	{
		return pSprite;
	}else
	{
		pSprite->Release();
		return NULL;
	}
}
