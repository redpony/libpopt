// Copyright (c) 2009 by Chris Dyer <redpony@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _OPTIMIZE_H_
#define _OPTIMIZE_H_

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include "lbfgs.h"

// abstract base class for first order optimizers
// order of invocation: new, Load(), Optimize(), Save(), delete
class Optimizer {
 public:
  Optimizer() : eval_(1), has_converged_(false) {}
  virtual ~Optimizer();
  virtual std::string Name() const = 0;
  int EvaluationCount() const { return eval_; }
  bool HasConverged() const { return has_converged_; }

  void Optimize(const double& obj,
                const std::vector<double>& g,
                std::vector<double>* x) {
    assert(g.size() == x->size());
    OptimizeImpl(obj, g, x);
    scitbx::lbfgs::traditional_convergence_test<double> converged(g.size());
    has_converged_ = converged(&(*x)[0], &g[0]);
  }

  void Save(std::ostream* out) const;
  void Load(std::istream* in);
 protected:
  virtual void SaveImpl(std::ostream* out) const;
  virtual void LoadImpl(std::istream* in);
  virtual void OptimizeImpl(const double& obj,
                            const std::vector<double>& g,
                            std::vector<double>* x) = 0;

  int eval_;
 private:
  bool has_converged_;
};

class RPropOptimizer : public Optimizer {
 public:
  explicit RPropOptimizer(int num_vars,
                          double eta_plus = 1.2,
                          double eta_minus = 0.5,
                          double delta_0 = 0.1,
                          double delta_max = 50.0,
                          double delta_min = 1e-6) :
      prev_g_(num_vars, 0.0),
      delta_ij_(num_vars, delta_0),
      eta_plus_(eta_plus),
      eta_minus_(eta_minus),
      delta_max_(delta_max),
      delta_min_(delta_min) {
    assert(eta_plus > 1.0);
    assert(eta_minus > 0.0 && eta_minus < 1.0);
    assert(delta_max > 0.0);
    assert(delta_min > 0.0);
  }
  std::string Name() const;
  void OptimizeImpl(const double& obj,
                    const std::vector<double>& g,
                    std::vector<double>* x);
  void SaveImpl(std::ostream* out) const;
  void LoadImpl(std::istream* in);
 private:
  std::vector<double> prev_g_;
  std::vector<double> delta_ij_;
  const double eta_plus_;
  const double eta_minus_;
  const double delta_max_;
  const double delta_min_;
};

class SGDOptimizer : public Optimizer {
 public:
  explicit SGDOptimizer(int num_vars, double eta = 0.1) : eta_(eta) {
    (void) num_vars;
  }
  std::string Name() const;
  void OptimizeImpl(const double& obj,
                    const std::vector<double>& g,
                    std::vector<double>* x);
 private:
  const double eta_;
};

class LBFGSOptimizer : public Optimizer {
 public:
  explicit LBFGSOptimizer(int num_vars, int memory_buffers = 10);
  std::string Name() const;
  void SaveImpl(std::ostream* out) const;
  void LoadImpl(std::istream* in);
  void OptimizeImpl(const double& obj,
                    const std::vector<double>& g,
                    std::vector<double>* x);
 private:
  scitbx::lbfgs::minimizer<double> opt_;
};

#endif
