#include <stdio.h>
#include <stdlib.h>

#include "thread_pool.h"

ThreadPool::ThreadPool(uint num_threads):
	thread_pool_(NULL),
	arr_work_threads_(nullptr),
	arr_callback_context_(nullptr),
	num_work_threads_(0),
	cleanup_group_(NULL),
	callback_environment_(),
	kNumThreads(num_threads)
{

}

ThreadPool::~ThreadPool(){
	for (uint i = 0; i < num_work_threads_; i++){
		CloseThreadpoolWork(arr_work_threads_[i]);
	}

	DestroyThreadpoolEnvironment(&callback_environment_);

	if(cleanup_group_){
		CloseThreadpoolCleanupGroup(cleanup_group_);
	}

	if(thread_pool_){
		CloseThreadpool(thread_pool_);
	}

	if(arr_work_threads_){
		free(arr_work_threads_);
	}

	if(arr_callback_context_){
		delete[] arr_callback_context_;
	}

}

bool ThreadPool::Initialize(){
	if(!InitializeThreadPool()){
		return false;
	}

	if(InitializaThreadPoolWork()){
		return false;
	}

	return true;
}

bool ThreadPool::InitializeThreadPool(){
	if(!kNumThreads)
		return false;

	thread_pool_ = CreateThreadpool(NULL);
	if(!thread_pool_){
		return false;
	}

	SetThreadpoolThreadMaximum (thread_pool_, kNumThreads);
	if(!SetThreadpoolThreadMinimum(thread_pool_, kNumThreads)){
		return false;
	}

	InitializeThreadpoolEnvironment(&callback_environment_);

	SetThreadpoolCallbackPool(&callback_environment_, thread_pool_);

	cleanup_group_ = CreateThreadpoolCleanupGroup();
	if(cleanup_group_){
		return false;
	}

	SetThreadpoolCallbackCleanupGroup(&callback_environment_, cleanup_group_, NULL);

	return true;
}

bool ThreadPool::InitializaThreadPoolWork(){
	arr_work_threads_ = (PTP_WORK*) malloc(sizeof(PTP_WORK) * kNumThreads);
	if(!arr_work_threads_){
		return false;
	}

	arr_callback_context_ = new CallbackProps[kNumThreads]();
	if(!arr_callback_context_){
		return false;
	}

	for (int i = 0; i < kNumThreads; i++){
		arr_work_threads_[i] = CreateThreadpoolWork(WorkCallback, (void*)&arr_callback_context_[i], &callback_environment_);
		if(!arr_work_threads_[i]){
			return false;
		}
		num_work_threads_++;
	}

	return true;
}

void CALLBACK ThreadPool::WorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work){
	CallbackProps* callback = (CallbackProps*) context;
	callback->callback_(callback->calback_arg_);
}