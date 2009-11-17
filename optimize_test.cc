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

#include <cassert>
#include <iostream>
#include <sstream>
#include "optimize.h"

using namespace std;

double TestOptimizer(Optimizer* opt) {
  cerr << "TESTING NON-PERSISTENT OPTIMIZER\n";

  // f(x,y) = 4x1^2 + x1*x2 + x2^2 + x3^2 + 6x3 + 5
  // df/dx1 = 8*x1 + x2
  // df/dx2 = 2*x2 + x1
  // df/dx3 = 2*x3 + 6
  vector<double> x(3);
  vector<double> g(3);
  x[0] = 8;
  x[1] = 8;
  x[2] = 8;
  double obj = 0;
  do {
    g[0] = 8 * x[0] + x[1];
    g[1] = 2 * x[1] + x[0];
    g[2] = 2 * x[2] + 6;
    obj = 4 * x[0]*x[0] + x[0] * x[1] + x[1]*x[1] + x[2]*x[2] + 6 * x[2] + 5;
    opt->Optimize(obj, g, &x);

    cerr << x[0] << " " << x[1] << " " << x[2] << endl;
    cerr << "   obj=" << obj << "\td/dx1=" << g[0] << " d/dx2=" << g[1] << " d/dx3=" << g[2] << endl;
  } while (!opt->HasConverged());
  return obj;
}

double TestPersistentOptimizer(Optimizer* opt) {
  cerr << "\nTESTING PERSISTENT OPTIMIZER\n";
  // f(x,y) = 4x1^2 + x1*x2 + x2^2 + x3^2 + 6x3 + 5
  // df/dx1 = 8*x1 + x2
  // df/dx2 = 2*x2 + x1
  // df/dx3 = 2*x3 + 6
  vector<double> x(3);
  vector<double> g(3);
  x[0] = 8;
  x[1] = 8;
  x[2] = 8;
  double obj = 0;
  string state;
  bool converged = false;
  while (!converged) {
    g[0] = 8 * x[0] + x[1];
    g[1] = 2 * x[1] + x[0];
    g[2] = 2 * x[2] + 6;
    obj = 4 * x[0]*x[0] + x[0] * x[1] + x[1]*x[1] + x[2]*x[2] + 6 * x[2] + 5;

    {
      if (state.size() > 0) {
        istringstream is(state, ios::binary);
        opt->Load(&is);
      }
      opt->Optimize(obj, g, &x);
      ostringstream os(ios::binary); opt->Save(&os); state = os.str();

    }

    cerr << x[0] << " " << x[1] << " " << x[2] << endl;
    cerr << "   obj=" << obj << "\td/dx1=" << g[0] << " d/dx2=" << g[1] << " d/dx3=" << g[2] << endl;
    converged = opt->HasConverged();
    if (!converged) {
      // now screw up the state (should be undone by Load)
      obj += 2.0;
      g[1] = -g[2];
      vector<double> x2 = x;
      try {
        opt->Optimize(obj, g, &x2);
      } catch (...) { }
    }
  }
  return obj;
}

template <class O>
void TestOptimizerVariants(int num_vars) {
  O oa(num_vars);
  const string name = oa.Name();
  cerr << "-------------------------------------------------------------------------\n";
  cerr << "TEST: " << name << endl;
  double o1 = TestOptimizer(&oa);
  O ob(num_vars);
  double o2 = TestPersistentOptimizer(&ob);
  if (o1 != o2) {
    cerr << oa.Name() << ": PERSISTENT/NON-PERSISTENT VARIANTS PERFORMED DIFFERENTLY!\n"
         << o1 << " vs. " << o2 << endl;
    exit(1);
  }
  cerr << oa.Name() << ": SUCCESS\n";
}

int main() {
  int n = 3;
  TestOptimizerVariants<SGDOptimizer>(n);
  TestOptimizerVariants<LBFGSOptimizer>(n);
  TestOptimizerVariants<RPropOptimizer>(n);
  cerr << "ALL OPTIMIZERS SUCCEEDED\n";
  return 0;
}

