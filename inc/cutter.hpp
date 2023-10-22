#pragma once
#include "millable.hpp"

namespace mini {
	class milling_cutter final {
		private:
			millable_block::milling_mask_t m_mask;
			int m_test;
			
		public:
			milling_cutter();
			~milling_cutter() = default;

			void update(const float delta_time, millable_block& block);
	};
}