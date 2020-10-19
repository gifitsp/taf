#pragma once

#include "../../../../libs/tbb/include/common/utility/utility.h"
#include "../../../../libs/tbb/include/common/utility/get_default_num_threads.h"
#include "../../../../libs/tbb/include/tbb/tbb.h"

#pragma warning(disable:4595 5040)

namespace taf::parellel
{
	/// <summary>
	/// detect total threads number supported by the machine
	/// </summary>
	/// <returns></returns>
	inline int get_core_threads_num()
	{
		return utility::get_default_num_threads();
	}

	/// <summary>
	/// parellel loop
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<class T>
	class Parellel
	{
		std::function<void(T&)> _func;

	public:
		Parellel(const std::function<void(T&)>& func)
		{
			_func = func;
		}

		inline void operator()(const tbb::blocked_range<T>& range) const
		{
			for (T p = range.begin(); p!= range.end(); ++p)
			{
				_func(p);
			}
		}

		inline void do_parallel(const T& begin, const T& end, int max_thread_num = get_core_threads_num())
		{
			tbb::global_control c(tbb::global_control::max_allowed_parallelism, max_thread_num);
			parallel_for(tbb::blocked_range(begin, end), *this);
		}
	};

	/// <summary>
	/// parellel task manager
	/// </summary>
	class Tasks
	{
		tbb::task_group _tg;

	public:
		template<typename F>
		inline void run(F f)
		{
			_tg.run(f);
		}

		inline void wait()
		{
			_tg.wait();
		}

		inline void cancel()
		{
			_tg.cancel();
		}

		template<typename...Funcs>
		static inline void run_and_wait_multiple(Funcs...funcs)
		{
			tbb::parallel_invoke(funcs...);
		}
	};

	class ThreadManager
	{
		std::vector<std::shared_ptr<std::thread>> _threads;

	public:
		template<typename F>
		inline void run(F f)
		{
			_threads.push_back(std::shared_ptr<std::thread>(new std::thread(f)));
		}

		inline void wait()
		{
			for (int i = 0; i < _threads.size(); ++i)
			{
				if (_threads[i]->joinable() && _threads[i]->get_id() != std::this_thread::get_id())
					_threads[i]->join();
			}
		}

		inline void cancel()
		{
		}
	};

	class IPipeline
	{
	};

	/// <summary>
	/// Input, Processor and Output should derive from below clases, and implement required functions
	/// </summary>
	/// <typeparam name="Output"></typeparam>
	/// <typeparam name="Input"></typeparam>
	/// <typeparam name="Processor"></typeparam>
	/// <typeparam name="Data"></typeparam>
	template<class Input, class Processor, class Output, typename Data>
	class Pipeline : public IPipeline
	{
		Input _input;
		Processor _processor;
		Output _output;
		int _max_thread_num = 1;

	public:
		Pipeline(int max_thread_num = get_core_threads_num())
		{
			_max_thread_num = max_thread_num;

			_input.pipeline = this;
			_processor.pipeline = this;
			_output.pipeline = this;
		}

		inline Input& get_input()
		{
			return _input;
		}

		inline Processor& get_processor()
		{
			return _processor;
		}

		inline Output& get_output()
		{
			return _output;
		}

		/// <summary>
		/// will block current thread
		/// </summary>
		inline void run()
		{
			tbb::parallel_pipeline(
				_max_thread_num,
				tbb::make_filter<void, Data>(
					tbb::filter::serial_in_order, _input)
				&
				tbb::make_filter<Data, Data>(
					tbb::filter::parallel, _processor)
				&
				tbb::make_filter<Data, void>(
					tbb::filter::serial_in_order, _output)
			);
		}
	};

	class FuncBase
	{
	public:
		IPipeline* pipeline;
	};

	/// <summary>
	/// T is the subclass
	/// </summary>
	/// <typeparam name="Data"></typeparam>
	/// <typeparam name="T"></typeparam>
	template<class T, typename Data>
	class InputFunc : public FuncBase
	{
	protected:
		volatile mutable bool _stop = false;

	public:
		inline void stop() const
		{
			_stop = true;
		}

		Data operator()(tbb::flow_control& fc) const
		{
			auto data = ((T*)this)->input();
			if (_stop)
				fc.stop();
			return data;
		}
	};

	template<class T, typename Data>
	class ProcessorFunc : public FuncBase
	{
	public:
		Data operator()(Data data) const
		{
			return ((T*)this)->process(data);
		}
	};

	template<class T, typename Data>
	class OutputFunc : public FuncBase
	{
	public:
		void operator()(Data data) const
		{
			((T*)this)->output(data);
		}
	};

#define fast_alloc tbb::cache_aligned_allocator
}

#ifdef USE_MEMPOOL
#include "../../../../libs/tbb/include/tbb/scalable_allocator.h"

_force_inline
void* operator new (size_t size)
#ifdef WINDOWS
throw (std::bad_alloc)
#endif
{
	if (_unlikely(size == 0)) size = 1;

	void* ptr = scalable_malloc(size);
	if (_likely(ptr))
		return ptr;
	throw std::bad_alloc();
}

_force_inline
void* operator new[](size_t size)
#ifdef WINDOWS
throw (std::bad_alloc)
#endif
{
	return operator new (size);
}

_force_inline
void* operator new (size_t size, const std::nothrow_t&)
#ifdef WINDOWS
throw ()
#endif
{
	if (_unlikely(size == 0)) size = 1;
	void* ptr = scalable_malloc(size);
	if (_likely(ptr))
		return ptr;
	return NULL;
}

_force_inline
void* operator new[](size_t size, const std::nothrow_t&)
#ifdef WINDOWS
throw ()
#endif
{
	return operator new (size, std::nothrow);
}

_force_inline
void operator delete (void* ptr)
#ifdef WINDOWS
throw ()
#endif
{
	if (_unlikely(ptr != 0)) scalable_free(ptr);
}

_force_inline
void operator delete[](void* ptr)
#ifdef WINDOWS
throw ()
#endif
{
	operator delete (ptr);
}

_force_inline
void operator delete (void* ptr, const std::nothrow_t&)
#ifdef WINDOWS
throw ()
#endif
{
	if (_unlikely(ptr != 0)) scalable_free(ptr);
}

_force_inline
void operator delete[](void* ptr, const std::nothrow_t&)
#ifdef WINDOWS
throw ()
#endif
{
	operator delete (ptr, std::nothrow);
}
#endif