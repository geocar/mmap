#include <node.h>
#include <node_buffer.h>

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

struct hint_wrap {
	size_t length;
};


static void Map_finalise(char *data, void*hint_void)
{
	struct hint_wrap *h = (struct hint_wrap *)hint_void;

	if(h->length > 0) {
		munmap(data, h->length);
	}
	delete h;
}

void Node_Sync(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto *isolate = args.GetIsolate();
	auto buffer = args.This()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
	char *data = node::Buffer::Data(static_cast<v8::Local<v8::Object>>(buffer));
	size_t length = node::Buffer::Length(buffer);

	// First optional argument: offset
	if (args.Length() > 0) {
		const size_t offset = args[0]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if(length <= offset) return;

		data += offset;
		length -= offset;
	}

	// Second optional argument: length
	if (args.Length() > 1) {
		const size_t range = args[1]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if(range < length) length = range;
	}

	// Third optional argument: flags
	int flags;
	if (args.Length() > 2) {
		flags = args[2]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	} else {
		flags = MS_SYNC;
	}

	args.GetReturnValue().Set((0 == msync(data, length, flags)) ? v8::True(isolate) : v8::False(isolate));
}

void Node_Unmap(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto *isolate = args.GetIsolate();
	auto context = isolate->GetCurrentContext();
	auto buffer = args.This()->ToObject(context).ToLocalChecked();
	char *data = node::Buffer::Data(buffer);

	auto keyString = v8::String::NewFromUtf8(isolate,"mmap_dptr").ToLocalChecked();
	auto key = v8::Private::New(isolate, keyString);
	struct hint_wrap *d = (struct hint_wrap *)v8::External::Cast(*buffer->GetPrivate(context, key).ToLocalChecked())->Value();

	bool ok = true;

	if(d->length > 0 && -1 == munmap(data, d->length)) {
		ok = false;
	} else {
		d->length = 0;
		(void)buffer->CreateDataProperty(isolate->GetCurrentContext(),
			v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(),
			v8::Number::New(isolate, 0));
	}

	args.GetReturnValue().Set(ok? v8::True(isolate): v8::False(isolate));
}

void Node_Map(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto *isolate = args.GetIsolate();
	auto context = isolate->GetCurrentContext();

	if (args.Length() <= 3)
	{
		isolate->ThrowException(
			v8::Exception::Error(
				v8::String::NewFromUtf8(isolate, "mmap() takes 4 arguments: size, protection, flags, fd and offset.").ToLocalChecked()));
		return;
	}

	const size_t length  = args[0]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	const int protection = args[1]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	const int flags      = args[2]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	const int fd         = args[3]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	const off_t offset   = args[4]->ToInteger(isolate->GetCurrentContext()).ToLocalChecked()->Value();

	char* data = (char *) mmap(0, length, protection, flags, fd, offset);

	if(data == MAP_FAILED)
	{
		isolate->ThrowException(node::ErrnoException(isolate, errno, "mmap", ""));
		return;
	}

	struct hint_wrap *d = new hint_wrap;
	d->length = length;

	auto buffer = node::Buffer::New(isolate, data, length, Map_finalise, (void*)d).ToLocalChecked();
	auto buffer_object = buffer->ToObject(context).ToLocalChecked();
	auto UNMAP = v8::String::NewFromUtf8(isolate, "unmap").ToLocalChecked();
	auto SYNC = v8::String::NewFromUtf8(isolate, "sync").ToLocalChecked();
	auto MMAP_DPTR = v8::Private::New(isolate, v8::String::NewFromUtf8(isolate, "mmap_dptr").ToLocalChecked());
	auto UnmapFN = v8::FunctionTemplate::New(isolate, Node_Unmap)->GetFunction(context).ToLocalChecked();
	auto SyncFN = v8::FunctionTemplate::New(isolate, Node_Sync)->GetFunction(context).ToLocalChecked();
	auto MMAP_DPTR_COPY = v8::External::New(isolate, (void*)d);

	buffer_object->Set(context, UNMAP, UnmapFN);
	buffer_object->Set(context, SYNC, SyncFN);
	buffer_object->SetPrivate(context,MMAP_DPTR, MMAP_DPTR_COPY);

	args.GetReturnValue().Set(buffer);
}


static void RegisterModule(v8::Local<v8::Object> exports)
{
	const int PAGESIZE = sysconf(_SC_PAGESIZE);

	NODE_SET_METHOD(exports, "map", Node_Map);
	NODE_DEFINE_CONSTANT(exports, PROT_READ);
	NODE_DEFINE_CONSTANT(exports, PROT_WRITE);
	NODE_DEFINE_CONSTANT(exports, PROT_EXEC);
	NODE_DEFINE_CONSTANT(exports, PROT_NONE);
	NODE_DEFINE_CONSTANT(exports, MAP_SHARED);
	NODE_DEFINE_CONSTANT(exports, MAP_PRIVATE);
	NODE_DEFINE_CONSTANT(exports, PAGESIZE);
	NODE_DEFINE_CONSTANT(exports, MS_ASYNC);
	NODE_DEFINE_CONSTANT(exports, MS_SYNC);
	NODE_DEFINE_CONSTANT(exports, MS_INVALIDATE);
}

NODE_MODULE(mmap, RegisterModule);
