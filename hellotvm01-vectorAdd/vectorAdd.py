"""
Get Started with TVM
====================
**Author**: `Tianqi Chen <https://tqchen.github.io>`_

This is an introduction tutorial to TVM.
TVM is a domain specific language for efficient kernel construction.

In this tutorial, we will demonstrate the basic workflow in TVM.
"""
from __future__ import absolute_import, print_function

import tvm
import numpy as np

######################################################################
# Vector Add Example
# ------------------
# In this tutorial, we will use a vector addition example to demonstrate
# the workflow.
#

######################################################################
# Describe the Computation
# ------------------------
# As a first step, we need to describe our computation.
# TVM adopts tensor semantics, with each intermediate result
# represented as multi-dimensional array. The user need to describe
# the computation rule that generate the tensors.
#
# We first define a symbolic variable n to represent the shape.
# We then define two placeholder Tensors, A and B, with given shape (n,)
#
# We then describe the result tensor C, with a compute operation.
# The compute function takes the shape of the tensor, as well as a lambda function
# that describes the computation rule for each position of the tensor.
#
# No computation happens during this phase, as we are only declaring how
# the computation should be done.
#
n = tvm.var("n")
A = tvm.placeholder((n,), name='A')
B = tvm.placeholder((n,), name='B')
C = tvm.compute(A.shape, lambda i: A[i] + B[i], name="C")
print(type(C))

######################################################################
# Schedule the Computation
# ------------------------
# While the above lines describes the computation rule, we can compute
# C in many ways since the axis of C can be computed in data parallel manner.
# TVM asks user to provide a description of computation called schedule.
#
# A schedule is a set of transformation of computation that transforms
# the loop of computations in the program.
#
# After we construct the schedule, by default the schedule computes
# C in a serial manner in a row-major order.
#
# .. code-block:: c
#
#   for (int i = 0; i < n; ++i) {
#     C[i] = A[i] + B[i];
#   }
#
s = tvm.create_schedule(C.op)

######################################################################
# We used the split construct to split the first axis of C,
# this will split the original iteration axis into product of
# two iterations. This is equivalent to the following code.
#
# .. code-block:: c
#
#   for (int bx = 0; bx < ceil(n / 64); ++bx) {
#     for (int tx = 0; tx < 64; ++tx) {
#       int i = bx * 64 + tx;
#       if (i < n) {
#         C[i] = A[i] + B[i];
#       }
#     }
#   }
#
bx, tx = s[C].split(C.op.axis[0], factor=64)

######################################################################
# Finally we bind the iteration axis bx and tx to threads in the GPU
# compute grid. These are GPU specific constructs that allows us
# to generate code that runs on GPU.
#
s[C].bind(bx, tvm.thread_axis("blockIdx.x"))
s[C].bind(tx, tvm.thread_axis("threadIdx.x"))

######################################################################
# Generate OpenCL Code
# --------------------
# TVM provides code generation features into multiple backends,
# we can also generate OpenCL code or LLVM code that runs on CPU backends.
#
# The following codeblocks generate opencl code, creates array on opencl
# device, and verifies the correctness of the code.
#
fadd_cl = tvm.build(s, [A, B, C], "opencl", name="myadd")
print("------opencl code------")
print(fadd_cl.imported_modules[0].get_source())
ctx = tvm.cl(0)
n = 1024
a = tvm.nd.array(np.random.uniform(size=n).astype(A.dtype), ctx)
b = tvm.nd.array(np.random.uniform(size=n).astype(B.dtype), ctx)
c = tvm.nd.array(np.zeros(n, dtype=C.dtype), ctx)
fadd_cl(a, b, c)
np.testing.assert_allclose(c.asnumpy(), a.asnumpy() + b.asnumpy())

######################################################################
# Summary
# -------
# This tutorial provides a walk through of TVM workflow using
# a vector add example. The general workflow is
#
# - Describe your computation via series of operations.
# - Describe how we want to compute use schedule primitives.
# - Compile to the target function we want.
# - Optionally, save the function to be loaded later.
#
# You are more than welcomed to checkout other examples and
# tutorials to learn more about the supported operations, schedule primitives
# and other features in TVM.
#
