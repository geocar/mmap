#include <node.h>
#include <node_buffer.h>

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

static v8::Persistent<v8::String> length_symbol;
static v8::Persistent<v8::String> unmap_symbol;
static v8::Persistent<v8::String> sync_symbol;
static v8::Persistent<v8::String> unlink_symbol;
static v8::Persistent<v8::String> name_symbol;
static v8::Persistent<v8::String> buffer_symbol;

static v8::Persistent<v8::Function> map_symbol;

static void Map_finalise(char *data, void*hint)
{
	munmap(data, (size_t)hint);
}

v8::Handle<v8::Value> Sync(const v8::Arguments& args)
{
	v8::HandleScope scope;

	node::Buffer *buffer = node::ObjectWrap::Unwrap<node::Buffer>(args.This()->GetHiddenValue(buffer_symbol)->ToObject());

	char* data = static_cast<char*>(buffer->handle_->GetIndexedPropertiesExternalArrayData());
	size_t length = buffer->handle_->GetIndexedPropertiesExternalArrayDataLength();

	// First optional argument: offset
	if (args.Length() > 0) {
		const size_t offset = args[0]->ToInteger()->Value();
		if(length <= offset) return v8::Undefined();

		data += offset;
		length -= offset;
	}

	// Second optional argument: length
	if (args.Length() > 1) {
		const size_t range = args[1]->ToInteger()->Value();
		if(range < length) length = range;
	}

	// Third optional argument: flags
	int flags;
	if (args.Length() > 2) {
		flags = args[2]->ToInteger()->Value();
	} else {
		flags = MS_SYNC;
	}

	if(0 == msync(data, length, flags)) {
		return v8::True();
	}

	return v8::False();
}

v8::Handle<v8::Value> Unmap(const v8::Arguments& args)
{
	v8::HandleScope scope;

	node::Buffer *buffer = node::ObjectWrap::Unwrap<node::Buffer>(args.This()->GetHiddenValue(buffer_symbol)->ToObject());

	char* data = static_cast<char*>(buffer->handle_->GetIndexedPropertiesExternalArrayData());
	size_t length = buffer->handle_->GetIndexedPropertiesExternalArrayDataLength();

	if(-1 == munmap(data, length)) return v8::False();

	buffer->handle_->SetIndexedPropertiesToExternalArrayData(NULL, v8::kExternalUnsignedByteArray, 0);
	buffer->handle_->Set(length_symbol, v8::Integer::NewFromUnsigned(0));
	buffer->handle_.Dispose();

	args.This()->Set(length_symbol, v8::Integer::NewFromUnsigned(0));

	return v8::True();
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

	node::Buffer *slowBuffer = node::Buffer::New(data, size, Map_finalise, (void *) size);

	v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();
	v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(buffer_symbol));
	v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer->handle_, args[0], v8::Integer::New(0) };
	v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

	actualBuffer->Set(unmap_symbol, v8::FunctionTemplate::New(Unmap)->GetFunction());
	actualBuffer->Set(sync_symbol, v8::FunctionTemplate::New(Sync)->GetFunction());
	actualBuffer->SetHiddenValue(buffer_symbol, slowBuffer->handle_);

	return scope.Close(actualBuffer);
}

v8::Handle<v8::Value> Unlink(const v8::Arguments& args)
{
	v8::HandleScope scope;
	v8::String::Utf8Value name(args.This()->GetHiddenValue(name_symbol)->ToString());
	if (shm_unlink(*name) < 0)
	{
		return v8::ThrowException(node::ErrnoException(errno, "unlink", ""));
	}
	return v8::True();
}

v8::Handle<v8::Value> MapShm(const v8::Arguments& args)
{
	v8::HandleScope scope;

	if (args.Length() <= 3)
	{
		return v8::ThrowException(
			v8::Exception::Error(
				v8::String::New("map_shm() takes 4 arguments: size, protection, flags, name and offset.")));
	}

	const size_t size    = args[0]->ToInteger()->Value();
	const v8::String::Utf8Value	name(args[3]->ToString());
	const int fd = shm_open(*name, O_CREAT | O_RDWR | O_SYNC, 0666);
  if (fd < 0)
  {
		return v8::ThrowException(node::ErrnoException(errno, "map_shm", ""));
  }
  ftruncate(fd, size);

  v8::Handle<v8::Value> *mapArgs = (v8::Handle<v8::Value> *)calloc(sizeof(v8::Handle<v8::Value>), args.Length());
  for (int i = 0; i < args.Length(); i++)
		mapArgs[i] = args[i];
  mapArgs[3] = v8::Integer::New(fd);

  v8::Handle<v8::Object> global = v8::Context::GetCurrent()->Global();
  v8::Handle<v8::Object> actualBuffer = map_symbol->Call(global, args.Length(), mapArgs)->ToObject();
  free(mapArgs);
  close(fd);

	actualBuffer->ToObject()->Set(unlink_symbol, v8::FunctionTemplate::New(Unlink)->GetFunction());
	actualBuffer->SetHiddenValue(name_symbol, v8::String::New(*name));

	return scope.Close(actualBuffer);
}

static void RegisterModule(v8::Handle<v8::Object> target)
{
	v8::HandleScope scope;

	length_symbol = NODE_PSYMBOL("length");
	sync_symbol   = NODE_PSYMBOL("sync");
	unmap_symbol  = NODE_PSYMBOL("unmap");
	unlink_symbol = NODE_PSYMBOL("unlink");
	name_symbol = NODE_PSYMBOL("name");
	buffer_symbol = NODE_PSYMBOL("Buffer");

	map_symbol = v8::Persistent<v8::Function>::New(v8::FunctionTemplate::New(Map)->GetFunction());

	const v8::PropertyAttribute attribs = (v8::PropertyAttribute) (v8::ReadOnly | v8::DontDelete);

	target->Set(v8::String::New("PROT_READ"), v8::Integer::New(PROT_READ), attribs);
	target->Set(v8::String::New("PROT_WRITE"), v8::Integer::New(PROT_WRITE), attribs);
	target->Set(v8::String::New("PROT_EXEC"), v8::Integer::New(PROT_EXEC), attribs);
	target->Set(v8::String::New("PROT_NONE"), v8::Integer::New(PROT_NONE), attribs);
	target->Set(v8::String::New("MAP_SHARED"), v8::Integer::New(MAP_SHARED), attribs);
	target->Set(v8::String::New("MAP_PRIVATE"), v8::Integer::New(MAP_PRIVATE), attribs);
	target->Set(v8::String::New("PAGESIZE"), v8::Integer::New(sysconf(_SC_PAGESIZE)), attribs);
	target->Set(v8::String::New("MS_ASYNC"), v8::Integer::New(MS_ASYNC), attribs);
	target->Set(v8::String::New("MS_SYNC"), v8::Integer::New(MS_SYNC), attribs);
	target->Set(v8::String::New("MS_INVALIDATE"), v8::Integer::New(MS_INVALIDATE), attribs);

	target->Set(v8::String::NewSymbol("map"),  v8::FunctionTemplate::New(Map)->GetFunction(), attribs);
	target->Set(v8::String::NewSymbol("map_shm"),  v8::FunctionTemplate::New(MapShm)->GetFunction(), attribs);
}

NODE_MODULE(mmap, RegisterModule);
