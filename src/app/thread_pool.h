#pragma once

#include <Windows.h>
#include <functional>

typedef unsigned int uint;

class ThreadPool{
public:
	ThreadPool(uint num_threads);
	~ThreadPool();

	struct CallbackProps{
		std::function<void(void*)> callback_;
		void *calback_arg_;
	};

	bool Initialize();

	inline uint GetNumCallbacks(){
		return num_work_threads_;
	}

	inline void SetCallback(std::function<void(void*)> callback, void *arg, uint callback_pos){
		if(callback_pos < num_work_threads_){
			arr_callback_context_[callback_pos].callback_ = callback;
			arr_callback_context_[callback_pos].calback_arg_ = arg;
		}
	}

	inline void Start(uint callback_pos){
		if(callback_pos < num_work_threads_){
			SubmitThreadpoolWork(arr_work_threads_[callback_pos]);
		}
	}

	inline void WaitAll(){
		for (uint i = 0; i < num_work_threads_; i++){
			WaitCallback(i);
		}
	}

	inline void WaitCallback(uint callback_pos){
		if(callback_pos < num_work_threads_){
			WaitForThreadpoolWorkCallbacks(arr_work_threads_[callback_pos], TRUE);
		}
	}

private:
	const int kNumThreads;

	PTP_POOL thread_pool_;
	PTP_WORK *arr_work_threads_;
	CallbackProps *arr_callback_context_;
	uint num_work_threads_;
	TP_CALLBACK_ENVIRON callback_environment_;
	PTP_CLEANUP_GROUP cleanup_group_;

	static void CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);

	bool InitializeThreadPool();
	bool InitializaThreadPoolWork();

};