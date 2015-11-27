#pragma once
#ifndef __CAVALIA_DATABASE_CERTIFY_WAIT_LOCK_H__
#define __CAVALIA_DATABASE_CERTIFY_WAIT_LOCK_H__

#include <cassert>
#include <SpinLock.h>

namespace Cavalia {
	namespace Database {
		class CertifyWaitLock {
		public:
			CertifyWaitLock() : reader_count_(0), is_writing_(false), is_certifying_(false){}
			~CertifyWaitLock(){}

			void AcquireReadLock() {
				while (1){
					while (is_certifying_ == true);
					spinlock_.Lock();
					if (is_certifying_ == true){
						spinlock_.Unlock();
					}
					else{
						++reader_count_;
						spinlock_.Unlock();
						return;
					}
				}
			}

			void AcquireWriteLock() {
				while (1){
					while (is_writing_ == true || is_certifying_ == true);
					spinlock_.Lock();
					if (is_writing_ == true || is_certifying_ == true){
						spinlock_.Unlock();
					}
					else{
						is_writing_ = true;
						spinlock_.Unlock();
						return;
					}
				}
			}

			void AcquireCertifyLock() {
				bool rt = true;
				while (1){
					while (reader_count_ != 0);
					spinlock_.Lock();
					if (reader_count_ != 0){
						spinlock_.Unlock();
					}
					else{
						is_writing_ = false;
						is_certifying_ = true;
						spinlock_.Unlock();
						return;
					}
				}
			}

			void ReleaseReadLock() {
				spinlock_.Lock();
				assert(reader_count_ > 0);
				--reader_count_;
				spinlock_.Unlock();
			}

			void ReleaseWriteLock() {
				spinlock_.Lock();
				assert(is_writing_ == true);
				is_writing_ = false;
				spinlock_.Unlock();
			}

			void ReleaseCertifyLock() {
				spinlock_.Lock();
				assert(is_certifying_ == true);
				is_certifying_ = false;
				spinlock_.Unlock();
			}

		private:
			SpinLock spinlock_;
			size_t reader_count_;
			bool is_writing_;
			bool is_certifying_;
		};
	}
}

#endif