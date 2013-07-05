#include <node.h>
#include <node_buffer.h>

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

static v8::Persistent<v8::String> oLength_symbol;
static v8::Persistent<v8::String> oUnmap_symbol;

static void Map_finalise(char *sData, void*xHint)
{
	munmap(sData, (size_t)xHint);
}

v8::Handle<v8::Value> Unmap(const v8::Arguments& oArgs)
{
	v8::HandleScope oScope;

	node::Buffer *oBuffer = node::ObjectWrap::Unwrap<node::Buffer>( oArgs.This() );

	char* sData = static_cast<char*>(oBuffer->handle_->GetIndexedPropertiesExternalArrayData());
	size_t iLength = oBuffer->handle_->GetIndexedPropertiesExternalArrayDataLength();

	oBuffer->handle_->SetIndexedPropertiesToExternalArrayData(NULL, v8::kExternalUnsignedByteArray, 0);
	oBuffer->handle_->Set(oLength_symbol, v8::Integer::NewFromUnsigned(0));
	oBuffer->handle_.Dispose();

	munmap(sData, iLength);

	return v8::Undefined();
}

v8::Handle<v8::Value> Map(const v8::Arguments& oArgs)
{
	v8::HandleScope oScope;

	if (oArgs.Length() <= 3)
	{
		return v8::ThrowException(
			v8::Exception::Error(
				v8::String::New("mmap() takes 4 arguments: size, protection, flags, fd and offset.")));
	}

	const size_t iSize    = oArgs[0]->ToInteger()->Value();
	const int iProtection = oArgs[1]->ToInteger()->Value();
	const int iFlags      = oArgs[2]->ToInteger()->Value();
	const int iFd         = oArgs[3]->ToInteger()->Value();
	const off_t iOffset   = oArgs[4]->ToInteger()->Value();

	char* sData = (char *) mmap(0, iSize, iProtection, iFlags, iFd, iOffset);

	if(sData == MAP_FAILED)
	{
		return v8::ThrowException(node::ErrnoException(errno, "mmap", ""));
	}

	node::Buffer *oBuffer = node::Buffer::New(sData, iSize, Map_finalise, (void *) iSize);
	oBuffer->handle_->Set(oUnmap_symbol, v8::FunctionTemplate::New(Unmap)->GetFunction());

	return oScope.Close( oBuffer->handle_ );
}


static void RegisterModule(v8::Handle<v8::Object> oTarget)
{
	v8::HandleScope oScope;

	oLength_symbol = NODE_PSYMBOL("length");
	oUnmap_symbol  = NODE_PSYMBOL("unmap");

	const v8::PropertyAttribute oAttribs = (v8::PropertyAttribute) (v8::ReadOnly | v8::DontDelete);

	oTarget->Set(v8::String::New("PROT_READ"), v8::Integer::New(PROT_READ), oAttribs);
	oTarget->Set(v8::String::New("PROT_WRITE"), v8::Integer::New(PROT_WRITE), oAttribs);
	oTarget->Set(v8::String::New("PROT_EXEC"), v8::Integer::New(PROT_EXEC), oAttribs);
	oTarget->Set(v8::String::New("PROT_NONE"), v8::Integer::New(PROT_NONE), oAttribs);
	oTarget->Set(v8::String::New("MAP_SHARED"), v8::Integer::New(MAP_SHARED), oAttribs);
	oTarget->Set(v8::String::New("MAP_PRIVATE"), v8::Integer::New(MAP_PRIVATE), oAttribs);
	oTarget->Set(v8::String::New("PAGESIZE"), v8::Integer::New(sysconf(_SC_PAGESIZE)), oAttribs);

	oTarget->Set(v8::String::NewSymbol("map"),  v8::FunctionTemplate::New(Map)->GetFunction(), oAttribs);
}

NODE_MODULE(mmap, RegisterModule);
