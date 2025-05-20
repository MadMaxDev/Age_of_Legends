#ifndef __PS_RESTORE_H__
#define __PS_RESTORE_H__

#include "scriptengine.h"
#include "context.h"

class psCRestore
{
public:
	psCRestore(psIBinaryStream* _stream, psCScriptEngine* _engine ):m_pStream(_stream), m_pModule(NULL),m_pEngine(_engine) {}
public:
	int SaveModule( psCModule* module );
	int RestoreModule(psCModule* module);

	int SaveEngineConfig( psCScriptEngine* engine) ;
	int RestoreEngineConfig(psCScriptEngine* engine);

	void WriteObjectTypeIdx(const psCObjectType* ot);	
	psCObjectType* ReadObjectTypeIdx();

	inline void Read(void* ptr,  psUINT size) { m_pStream->Read(ptr, size); }
	inline void Write(const void* ptr, psUINT size) { m_pStream->Write(ptr, size); }
protected:
	psIBinaryStream* m_pStream;
	psCModule*		 m_pModule;
	psCScriptEngine* m_pEngine;
};

#endif // __PS_SCRIPTCODE_H__
