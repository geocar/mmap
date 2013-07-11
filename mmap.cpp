#include <node.h>
#include <node_buffer.h>

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

static v8::Persistent<v8::String> length_symbol;
static v8::Persistent<v8::String> unmap_symbol;

static void Map_finalise(char *data, void*hint)
{
	munmap(data, (size_t)hint);
}

v8::Handle<v8::Value> Unmap(const v8::Arguments& args)
{
	v8::HandleScope scope;

	node::Buffer *buffer = node::ObjectWrap::Unwrap<node::Buffer>( args.This() );

	char* data = static_cast<char*>(buffer->handle_->GetIndexedPropertiesExternalArrayData());
	size_t length = buffer->handle_->GetIndexedPropertiesExternalArrayDataLength();

	buffer->handle_->SetIndexedPropertiesToExternalArrayData(NULL, v8::kExternalUnsignedByteArray, 0);
	buffer->handle_->Set(length_symbol, v8::Integer::NewFromUnsigned(0));
	buffer->handle_.Dispose();

	munmap(data, length);

	return v8::Undefined();
}

v8::Handle<v8::Value> Map(const v8::Arguments& args)
{
	v8::HandleScope scope;

	if (args.Length() <= 3)
	{
		return v8::ThrowException(
			v8::Exception::Error(
				v8::String::New("mmap() takes 4 arguments: size, protection, flags, fd and offset.")));
	}

	const size_t size    = args[0]->ToInteger()->Value();
	const int protection = args[1]->ToInteger()->Value();
	const int flags      = args[2]->ToInteger()->Value();
	const int fd         = args[3]->ToInteger()->Value();
	const off_t offset   = args[4]->ToInteger()->Value();

	char* data = (char *) mmap(0, size, protection, flags, fd, offset);

	if(data == MAP_FAILED)
	{
		return v8::ThrowException(node::ErrnoException(errno, "mmap", ""));
	}

	node::Buffer *buffer = node::Buffer::New(data, size, Map_finalise, (void *) size);
	buffer->handle_->Set(unmap_symbol, v8::FunctionTemplate::New(Unmap)->GetFunction());

	return scope.Close( buffer->handle_ );
}


static void RegisterModule(v8::Handle<v8::Object> target)
{
	v8::HandleScope scope;

	length_symbol = NODE_PSYMBOL("length");
	unmap_symbol  = NODE_PSYMBOL("unmap");

	const v8::PropertyAttribute attribs = (v8::PropertyAttribute) (v8::ReadOnly | v8::DontDelete);

	target->Set(v8::String::New("PROT_READ"), v8::Integer::New(PROT_READ), attribs);
	target->Set(v8::String::New("PROT_WRITE"), v8::Integer::New(PROT_WRITE), attribs);
	target->Set(v8::String::New("PROT_EXEC"), v8::Integer::New(PROT_EXEC), attribs);
	target->Set(v8::String::New("PROT_NONE"), v8::Integer::New(PROT_NONE), attribs);
	target->Set(v8::String::New("MAP_SHARED"), v8::Integer::New(MAP_SHARED), attribs);
	target->Set(v8::String::New("MAP_PRIVATE"), v8::Integer::New(MAP_PRIVATE), attribs);
	target->Set(v8::String::New("PAGESIZE"), v8::Integer::New(sysconf(_SC_PAGESIZE)), attribs);

	target->Set(v8::String::NewSymbol("map"),  v8::FunctionTemplate::New(Map)->GetFunction(), attribs);
}

NODE_MODULE(mmap, RegisterModule);
