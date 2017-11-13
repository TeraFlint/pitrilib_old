#pragma once

namespace Pitri
{
	struct IUnknown;

	template<class T>
	class ComPtr
	{
		private:
			T *ptr;

		public:
			explicit ComPtr(T *ptr = 0) : ptr(ptr)
			{
			}
			ComPtr(const ComPtr &other) : ptr(other.ptr)
			{
				if (ptr)
					ptr->AddRef();
			}
			~ComPtr()
			{
				if (ptr)
					ptr->Release();
			}
			ComPtr<T> &operator = (const ComPtr &other)
			{
				if (this != &other)
				{
					if (ptr)
						ptr->Release();
					ptr = other.ptr;
					if (ptr)
						ptr->AddRef();
				}
				return *this;
			}
			ComPtr<T> &operator = (T *other)
			{
				if (ptr)
					ptr->Release();
				ptr = other;
				return *this;
			}
			T *operator -> ()
			{
				return ptr;
			}
			T **operator & ()
			{
				return &ptr;
			}
			operator bool() const
			{
				return ptr != nullptr;
			}
			operator T *() const
			{
				return ptr;
			}
	};
}