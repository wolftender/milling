#include "cutter.hpp"

namespace mini {
	milling_cutter::milling_cutter() :
		m_mask(50, 50) {
		m_test = -50;

		std::fill(m_mask.mask.begin(), m_mask.mask.end(), 0.5f);
	}

	void milling_cutter::update(const float delta_time, millable_block& block) {
		m_test++;

		block.carve(m_mask, m_test, m_test, 0.0f, 0.0f);
	}
}