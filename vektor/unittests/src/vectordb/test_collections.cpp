// Copyright 2026, Elias Lopes
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Elias Lopes nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.


#include <gtest/gtest.h>
#include <string>

#include "vectordb/collections.hpp"



/**
 * @brief Test if a existing metric can be 
 *  converted to a string.
 * 
 */
TEST(MetricConversionHelpers, MetricToStringSuccess) {
    collections::metric _cosine         = collections::metric::cosine;
    collections::metric _l2             = collections::metric::l2;
    collections::metric _inner_product  = collections::metric::inner_product;

    // Check expected return value
    EXPECT_EQ(collections::metric_to_string(_cosine) ,        "cosine");
    EXPECT_EQ(collections::metric_to_string(_l2) ,            "l2");
    EXPECT_EQ(collections::metric_to_string(_inner_product) , "inner_product");
}

/**
 * @brief Test if non-existing metric 
 *  cannot be converted and throws exception.
 * 
 */
TEST(MetricConversionHelpers, MetricToStringNotFound) {
    auto m = static_cast<collections::metric>(999);
    // Check expected return value
    EXPECT_THROW(collections::metric_to_string(m) ,std::invalid_argument);
}


// ----------------------------------

/**
 * @brief Test successfully converting string
 *  into an exsisting metric.
 */
TEST(MetricConversionHelpers, StringToMetricSuccess) {
    EXPECT_EQ(collections::string_to_metric("cosine") ,         collections::metric::cosine);
    EXPECT_EQ(collections::string_to_metric("l2") ,             collections::metric::l2);
    EXPECT_EQ(collections::string_to_metric("inner_product") ,  collections::metric::inner_product);
}

/**
 * @brief Test fail when converting string
 *  into an undefined metric.
 */
TEST(MetricConversionHelpers, StringToMetricNotFound) {
    EXPECT_THROW(collections::string_to_metric("missing") , std::invalid_argument);
}




















