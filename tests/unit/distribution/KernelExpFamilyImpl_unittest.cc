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

#include <shogun/features/DenseFeatures.h>
#include <shogun/distributions/kernel_exp_family/impl/KernelExpFamilyImpl.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/mathematics/Math.h>
#include <shogun/base/some.h>
#include <gtest/gtest.h>

using namespace shogun;

TEST(KernelExpFamilyImpl, kernel_equals_manual)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=2;
	X(0,1)=4;
	X(1,1)=8;
	X(0,2)=3;
	X(1,2)=6;
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	for (auto idx_a=0; idx_a<N; idx_a++)
	{
		for (auto idx_b=0; idx_b<N; idx_b++)
		{
			float64_t k = 0;
			for (auto i=0; i<D; i++)
			{
				float64_t d = X(i,idx_a)-X(i,idx_b);
				k+=d*d;
			}
			k = exp(-k/sigma);

			EXPECT_NEAR(k, est.kernel(idx_a, idx_b), 1e-15);
		}
	}
}

TEST(KernelExpFamilyImpl, kernel_equals_shogun)
{
	index_t N=30;
	index_t D=20;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	auto k = new CGaussianKernel();
	SG_REF(k);
	auto f = new CDenseFeatures<float64_t>(X);
	SG_REF(f);
	k->set_width(sigma);
	k->init(f,f);
	
	for (auto idx_a=0; idx_a<N; idx_a++)
	{
		for (auto idx_b=0; idx_b<N; idx_b++)
			EXPECT_NEAR(k->kernel(idx_a, idx_b), est.kernel(idx_a, idx_b), 1e-15);
	}
	SG_UNREF(k);
	SG_UNREF(f);
}

TEST(KernelExpFamilyImpl, kernel_dx)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a = 0;
	SGVector<float64_t> b(D);
	b[0]=-1;
	b[1]=3;
	est.set_test_data(b);
	auto result = est.kernel_dx(idx_a, 0);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = {-0.082085, 0.16417 };
	ASSERT_EQ(result.vlen, D);
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(result.vector[i], reference[i], 1e-8);
	
	idx_a = 1;
	result = est.kernel_dx(idx_a, 0);
	float64_t reference2[] = {-0.02021384,  -0.00673795};
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(result.vector[i], reference2[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_dx_dx_dy)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a = 0;
	index_t idx_b = 1;
	auto result = est.kernel_dx_dx_dy(idx_a, idx_b);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = {-0.00300688, -0.02405503,
							 -0.01353095, -0.02706191};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
	
	idx_a = 0;
	idx_b = 0;
	result = est.kernel_dx_dx_dy(idx_a, idx_b);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], 0, 1e-8);
	
	idx_a = 1;
	idx_b = 1;
	result = est.kernel_dx_dx_dy(idx_a, idx_b);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], 0, 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_dx_dx_dy_dy)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	index_t idx_a = 0;
	index_t idx_b = 1;
	auto result = est.kernel_dx_dx_dy_dy(idx_a, idx_b);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = {-0.0075172, 0.03608254,
							 0.03608254 , 0.04510318};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
		
	idx_a = 0;
	idx_b = 0;
	result = est.kernel_dx_dx_dy_dy(idx_a, idx_b);
	float64_t reference2[] = { 3.,  1., 1.,  3.};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference2[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_hessian)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a = 0;
	index_t idx_b = 1;
	auto result = est.kernel_hessian(idx_a, idx_b);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = {-0.00451032, -0.00902064,
							 -0.00902064 , -0.01202751};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
		
	idx_a = 0;
	idx_b = 0;
	result = est.kernel_hessian(idx_a, idx_b);
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D; i++)
		for (auto j=0; i<D; i++)
		{
			float64_t ref;
			if (i==j)
				ref=1;
			else
				ref=0;
				
			EXPECT_EQ(result(i,j), ref);
		}
}

TEST(KernelExpFamilyImpl, kernel_dx_dx)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a=0;
	index_t idx_b=1;
	SGVector<float64_t> a(X.get_column_vector(idx_a), D, false);
	est.set_test_data(a);
	
	auto result = est.kernel_dx_dx(idx_b, idx_a);
	float64_t reference[] = { 0.00451032,  0.01202751};
	
	ASSERT_EQ(result.vlen, D);
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(result.vector[i], reference[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_dx_i_dx_i_dx_j)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a=0;
	index_t idx_b=1;
	SGVector<float64_t> a(X.get_column_vector(idx_a), D, false);
	
	est.set_test_data(a);
	auto result = est.kernel_dx_i_dx_i_dx_j(idx_b, 0);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = { -0.00300688,  -0.02405503, -0.01353095,  -0.02706191};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
	
	idx_b=2;
	result = est.kernel_dx_i_dx_i_dx_j(idx_b, 0);
	float64_t reference2[] = {-7.45188789e-07,   -2.98075516e-06, -1.65597509e-06,  -4.55393149e-06};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference2[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_dx_i_dx_j)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	index_t idx_a=0;
	index_t idx_b=1;
	SGVector<float64_t> a(X.get_column_vector(idx_a), D, false);
	
	est.set_test_data(a);
	auto result = est.kernel_dx_i_dx_j(idx_b, 0);
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = { 0.00451032,  0.00902064, 0.00902064,  0.01202751};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
	
	idx_b=2;
	result = est.kernel_dx_i_dx_j(idx_b, 0);
	float64_t reference2[] = { 3.31195018e-07,   6.20990658e-07,  6.20990658e-07,   9.93585053e-07};
	ASSERT_EQ(result.num_rows, D);
	ASSERT_EQ(result.num_cols, D);
	for (auto i=0; i<D*D; i++)
		EXPECT_NEAR(result.matrix[i], reference2[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_hessian_all)
{
	index_t N=3;
	index_t D=2;
	auto ND=N*D;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	auto result = est.kernel_hessian_all();
	
	// from kernel_exp_family Python implementation
	float64_t reference[] = {
		1.0 ,0.0 ,-0.00451031757893 ,-0.00902063515787 ,-3.31195017503e-07 ,-6.20990657818e-07 ,
		0.0 ,1.0 ,-0.00902063515787 ,-0.0120275135438 ,-6.20990657818e-07 ,-9.93585052508e-07 ,
		-0.00451031757893 ,-0.00902063515787 ,1.0 ,0.0 ,0.0 ,-0.164169997248 ,-0.00902063515787 ,
		-0.0120275135438 ,0.0 ,1.0 ,-0.164169997248 ,-0.246254995872 ,-3.31195017503e-07 ,
		-6.20990657818e-07 ,0.0 ,-0.164169997248 ,1.0 ,0.0 ,-6.20990657818e-07 ,-9.93585052508e-07 ,
		-0.164169997248 ,-0.246254995872 ,0.0 ,1.0 	};
							 
	ASSERT_EQ(result.num_rows, ND);
	ASSERT_EQ(result.num_cols, ND);
	for (auto i=0; i<ND*ND; i++)
		EXPECT_NEAR(result.matrix[i], reference[i], 1e-8);
}

TEST(KernelExpFamilyImpl, compute_h)
{
	index_t N=3;
	index_t D=2;
	auto ND = N*D;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	auto result = est.compute_h();

	// from kernel_exp_family Python implementation
	float64_t reference[] = {0.00902188,  0.01353302,  0.01834103,  0.04119238,
							 -0.02736291,-0.0547254 };
	ASSERT_EQ(result.vlen, ND);
	for (auto i=0; i<ND; i++)
		EXPECT_NEAR(result[i], reference[i], 1e-8);
}

TEST(KernelExpFamilyImpl, compute_xi_norm_2)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	auto result = est.compute_xi_norm_2();
	
	// from kernel_exp_family Python implementation
	EXPECT_NEAR(result, 2.5633762219921161, 1e-15);
}

TEST(KernelExpFamilyImpl, build_system)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	auto result = est.build_system();
	auto A = result.first;
	auto b = result.second;
	
	auto ND=N*D;
	ASSERT_EQ(A.num_rows, ND+1);
	ASSERT_EQ(A.num_cols, ND+1);
	ASSERT_EQ(b.vlen, ND+1);
	
	// from kernel_exp_family Python implementation
	float64_t reference_A[] = {
2.56539000231 ,0.0118777487667 ,0.0178237575117 ,0.0273952084559 ,0.0608313131137 ,
-0.0387380656692 ,-0.0773521682976 ,0.0118777487667 ,1.33336723827 ,4.97272472278e-05 ,
-0.00751716198221 ,-0.0150343228317 ,0.000493087224651 ,0.000986243448264 ,
0.0178237575117 ,4.97272472278e-05 ,1.33340867765 ,-0.0150343375575 ,-0.0200457403653 ,
0.000657150637366 ,0.00147926167395 ,0.0273952084559 ,-0.00751716198221 ,
-0.0150343375575 ,1.34235116761 ,0.0135256212451 ,2.3651749567e-09 ,-0.273616658158 ,
0.0608313131137 ,-0.0150343228317 ,-0.0200457403653 ,0.0135256212451 ,1.36260644798 ,
-0.273616658594 ,-0.410424987269 ,-0.0387380656692 ,0.000493087224651 ,0.000657150637366 ,
2.3651749567e-09 ,-0.273616658594 ,1.34231726267 ,0.0134758939984 ,-0.0773521682976 ,
0.000986243448264 ,0.00147926167395 ,-0.273616658158 ,-0.410424987269 ,0.0134758939984 ,
1.36253110366};


	float64_t reference_b[] = { -2.56337622, -0.00902188, -0.01353302, -0.01834103, -0.04119238,
								0.02736291,  0.0547254 };

	for (auto i=0; i<(ND+1)*(ND+1); i++)
		EXPECT_NEAR(A.matrix[i], reference_A[i], 1e-8);
	
	for (auto i=0; i<ND+1; i++)
		EXPECT_NEAR(b[i], reference_b[i], 1e-8);
	
}

TEST(KernelExpFamilyImpl, fit)
{
	index_t N=3;
	index_t D=2;
	auto ND=N*D;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	
	est.fit();
	auto x = est.get_alpha_beta();
	ASSERT_EQ(x.vlen, ND+1);
	ASSERT(x.vector);
	
	// from kernel_exp_family Python implementation
	float64_t reference_x[] = {-0.99999999999999989, 0.00228091,  0.00342023,
         0.00406425,  0.0092514 ,
        -0.00646103, -0.01294499};

	for (auto i=0; i<ND+1; i++)
		EXPECT_NEAR(x[i], reference_x[i], 1e-5);
	
}

TEST(KernelExpFamilyImpl, log_pdf)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	est.fit();
	
	SGVector<float64_t> x(D);
	x[0] = 0;
	x[1] = 1;
	auto log_pdf = est.log_pdf(x);

	// from kernel_exp_family Python implementation
	EXPECT_NEAR(log_pdf, 0.6612075586873365, 1e-15);
}

TEST(KernelExpFamilyImpl, grad)
{
	index_t N=3;
	index_t D=2;
	SGMatrix<float64_t> X(D,N);
	X(0,0)=0;
	X(1,0)=1;
	X(0,1)=2;
	X(1,1)=4;
	X(0,2)=3;
	X(1,2)=6;
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	est.fit();
	
	SGVector<float64_t> x(D);
	x[0] = 0;
	x[1] = 1;
	auto grad = est.grad(x);

	// from kernel_exp_family Python implementation
	float64_t reference[] = {-0.01120102, -0.01680534};
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(grad[i], reference[i], 1e-8);
	
	x[0] = 1;
	x[1] = 1;
	grad = est.grad(x);
	float64_t reference2[] = {-0.61982803, -0.04194253};
	for (auto i=0; i<D; i++)
		EXPECT_NEAR(grad[i], reference2[i], 1e-8);
}

TEST(KernelExpFamilyImpl, kernel_dx_dx_dy_dy_sum)
{
	index_t N=5;
	index_t D=3;
	SGMatrix<float64_t> X(D,N);
	for (auto i=0; i<N*D; i++)
		X.matrix[i]=CMath::randn_float();
		
	float64_t sigma = 2;
	float64_t lambda = 1;
	KernelExpFamilyImpl est(X, sigma, lambda);
	est.fit();
	
	// compare against batch version
	auto idx_a=0;
	auto idx_b=1;
	auto mat = est.kernel_dx_dx_dy_dy(idx_a, idx_b);
	float64_t sum_manual=0;
	for (auto i=0; i<D*D; i++)
		sum_manual += mat.matrix[i];
		
	auto sum = est.kernel_dx_dx_dy_dy_sum(idx_a, idx_b);
	EXPECT_NEAR(sum, sum_manual, 1e-14);
}
