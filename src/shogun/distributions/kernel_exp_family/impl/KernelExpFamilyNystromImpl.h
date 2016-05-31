/*
 * Copyright (c) The Shogun Machine Learning Toolbox
 * Written (w) 2016 Heiko Strathmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Shogun Development Team.
 */

#ifndef KERNEL_EXP_FAMILY_NYSTROM_IMPL__
#define KERNEL_EXP_FAMILY_NYSTROM_IMPL__

#include <shogun/lib/config.h>
#include <shogun/lib/common.h>
#include <shogun/lib/SGMatrix.h>
#include <shogun/lib/SGVector.h>

#include "KernelExpFamilyImpl.h"

namespace shogun
{

class KernelExpFamilyNystromImpl : public KernelExpFamilyImpl
{
public :
	KernelExpFamilyNystromImpl(SGMatrix<float64_t> data, float64_t sigma, float64_t lambda,
				index_t num_rkhs_basis, bool low_memory_mode=false);
	KernelExpFamilyNystromImpl(SGMatrix<float64_t> data, float64_t sigma, float64_t lambda,
			SGVector<index_t> inds, bool low_memory_mode=false);

	virtual ~KernelExpFamilyNystromImpl() {};

	float64_t difference_component(index_t idx_a, index_t idx_b, index_t i) const;

	// for training
	float64_t kernel_hessian_component(const index_t idx_a, index_t idx_b, index_t i, index_t j) const;
	virtual void sub_sample_rkhs_basis(index_t num_rkhs_basis);

	// for evaluation
	float64_t kernel_dx_component(index_t idx_a, index_t idx_b, index_t i) const;
	float64_t kernel_dx_dx_component(index_t idx_a, index_t idx_b, index_t i) const;
	SGVector<float64_t> kernel_dx_i_dx_i_dx_j_component(index_t idx_a, index_t idx_b, index_t i) const;
	SGVector<float64_t> kernel_dx_i_dx_j_component(index_t idx_a, index_t idx_b, index_t i) const;

	float64_t compute_lower_right_submatrix_element(index_t row_idx, index_t col_idx) const;
	SGVector<float64_t> compute_first_row_no_storing() const;
	virtual std::pair<SGMatrix<float64_t>, SGVector<float64_t>> build_system_slow_low_memory() const;
	virtual std::pair<SGMatrix<float64_t>, SGVector<float64_t>> build_system_fast_high_memory() const;

	virtual void fit();

	virtual float64_t log_pdf(index_t idx_test) const;
	virtual SGVector<float64_t> grad(index_t idx_test) const;

	// wtf: why is this necessary? But removing it causes compile error.
	float64_t log_pdf(SGVector<float64_t> x) { return KernelExpFamilyImpl::log_pdf(x); }
	SGVector<float64_t> grad(SGVector<float64_t> x) { return KernelExpFamilyImpl::grad(x); }

	std::pair<index_t, index_t> idx_to_ai(index_t idx) const;
	static SGMatrix<float64_t> pinv(const SGMatrix<float64_t>& A);

	SGVector<index_t> get_inds() { return m_inds; }


protected:
	index_t get_num_rkhs_basis() const;

protected:
	SGVector<index_t> m_inds;

	bool m_low_memory_mode;
};

}
#endif // KERNEL_EXP_FAMILY_NYSTROM_IMPL__
