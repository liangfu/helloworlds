/***************************************************************************
 *   Copyright (C) 2004 by Nicolas Brodu                                   *
 *   nicolas.brodu@free.fr                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

//#define MLP_LOG_BPSTEPS
// #include "Utility.h"
// using namespace Utility;

// int nhiddens, nbpsteps;
// double bprate;

// compiler use temporary instead of explicit variable
inline double square(double x) {
    return x*x;
}

// Uncomment this and see the code at the end of this file for debugging
// Trick not to have to modify the header to avoid recompiling dependent objects

//#define MULTILAYERPERCEPTRON_H_DEBUG_MODE     public: void gradCheck();


#include "MultiLayerPerceptron.h"

using namespace std;

const double MultiLayerPerceptron::defaultLearningRate = 0.1;

MultiLayerPerceptron::MultiLayerPerceptron(int nin, int nhid, int nout)
: weights(0), gradients(0), geneweights(0) {
    ninput = nin;
    nhidden = nhid;
    noutput = nout;

    nih = ninput * nhidden;
    nho = nhidden * noutput;
    geneweights.resize(nih+nho+nhidden+noutput);
}

MultiLayerPerceptron::MultiLayerPerceptron()
: weights(0), gradients(0), geneweights(0)
{
    // categorical variables set to +1/-1 for yes/no to match use of +1/-1 sigmoid (better numerical conditioning)
    ninput = ninputs * nsymbols;
    nhidden = nhiddens;
    //noutput = nsymbols; // categorical variables // but outputs are always trained on known data
    // and add one for "positive/negative" learning flag
    noutput = nsymbols+1;

    nih = ninput * nhidden;
    nho = nhidden * noutput;

    // genetic info remains in memory
    geneweights.resize(nih+nho+nhidden+noutput);

}

MultiLayerPerceptron::~MultiLayerPerceptron()
{
}

// non-genetic info may be flushed and re-init to save memory
void MultiLayerPerceptron::init()
{
    weights = geneweights;
    gradients.resize(weights.size());
    fill(gradients.begin(), gradients.end(), 0.0f);
    hiddenValues.resize(nhidden);

    // input to hidden weights: array of size ninput * nhidden
    ihw = &weights[0];
    // hidden to output weights: array of size nhidden * noutput
    how = &weights[nih];
    // hidden unit bias: array of size nhidden
    hb = &weights[nih+nho];
    // output unit bias: array of size output
    ob = &weights[nih+nho+nhidden];

    // input to hidden weight gradients: array of size ninput * nhidden
    ihwg = &gradients[0];
    // hidden to output weight gradients: array of size nhidden * noutput
    howg = &gradients[nih];
    // hidden unit bias gradients: array of size nhidden
    hbg = &gradients[nih+nho];
    // output unit bias gradients: array of size output
    obg = &gradients[nih+nho+nhidden];

    // hidden values holder.
    hv = &hiddenValues[0];

    trainingInstances.clear();
}

MultiLayerPerceptron::MultiLayerPerceptron(const MultiLayerPerceptron& mlp) {
    ninput  = mlp.ninput;
    nhidden = mlp.nhidden;
    noutput = mlp.noutput;
    nih = mlp.nih;
    nho = mlp.nho;

    // Copy memory arrays
    weights = mlp.weights;
    gradients = mlp.gradients;
    geneweights = mlp.geneweights;
    hiddenValues = mlp.hiddenValues;

    // Point aliases to our copies -- see init()
    ihw = &weights[0];
    how = &weights[nih];
    hb = &weights[nih+nho];
    ob = &weights[nih+nho+nhidden];
    ihwg = &gradients[0];
    howg = &gradients[nih];
    hbg = &gradients[nih+nho];
    obg = &gradients[nih+nho+nhidden];
    hv = &hiddenValues[0];

    // Training data set: map inputs to output symbol = knowledge is copied too
    trainingInstances = mlp.trainingInstances;
    // more genetic info
    initseq = mlp.initseq;
}


void MultiLayerPerceptron::unload() {

    Vec1d().swap(weights);
    Vec1d().swap(gradients);
    Vec1d().swap(hiddenValues);

    vector< pair<Vec1d, Vec1d> >().swap(trainingInstances);
}

void MultiLayerPerceptron::randCreateGeneticInfo() {
    initseq.resize(ninputs);
    // In our case, equiv to mutate with 100% proba
    mutate(100);
}


// clone genetic info
void MultiLayerPerceptron::clone(const MultiLayerPerceptron& parent) {
    geneweights = parent.geneweights;
    initseq = parent.initseq;
}

void MultiLayerPerceptron::crossover(const MultiLayerPerceptron& parent1, const MultiLayerPerceptron& parent2) {

    // preserve logical groups of weights & choose from one parent or the other

    // input-to-hidden
    int gbase = 0;
    for (int h=0; h<nhidden; ++h) {
        const MultiLayerPerceptron& parent = (Random()<0.5) ? parent1 : parent2;
        // copy the parent weight row
        for (int i=0; i<ninputs; ++i) geneweights[gbase+i] = parent.geneweights[gbase+i];
        // copy the matching bias
        geneweights[nih+nho+h] = parent.geneweights[nih+nho+h];
        gbase += ninputs;
    }

    // hidden-to-output
    for (int o=0; o<noutput; ++o) {
        const MultiLayerPerceptron& parent = (Random()<0.5) ? parent1 : parent2;
        // copy the parent weight row
        for (int h=0; h<nhidden; ++h) geneweights[gbase+h] = parent.geneweights[gbase+h];
        // copy the matching bias
        geneweights[nih+nho+nhidden+o] = parent.geneweights[nih+nho+nhidden+o];
        gbase += nhidden;
    }

    // initseq is taken from both parents
    initseq.resize(ninputs);
    for (int i=0; i<ninputs; ++i) {
        initseq[i] = (BRandom() & 1) ? parent1.initseq[i] : parent2.initseq[i];
    }
}

void MultiLayerPerceptron::mutate(int mutationPercentProba) {
/*
    double scale = 1.0 / sqrt(ninput+1.0);
    for (int i=0; i<nih; ++i) if (rand()%100<mutationPercentProba) geneweights[i] = NRandom() * scale;
    scale = 1.0 / sqrt(nhidden+1.0);
    for (int i=0; i<nho+nhidden; ++i) if (rand()%100<mutationPercentProba) geneweights[nih+i] = NRandom() * scale;
    scale = 1.0 / sqrt(noutput+1.0);
    for (int i=0; i<noutput; ++i) if (rand()%100<mutationPercentProba) geneweights[nih+nho+nhidden+i] = NRandom() * scale;

    for (int i=0; i<ninputs; ++i) if (rand()%100<mutationPercentProba) initseq[i] = rand() % nsymbols;
*/
    double pr = mutationPercentProba * 0.01;
    double scale = pr / sqrt(ninput+1.0);
    for (int i=0; i<nih; ++i) geneweights[i] += NRandom() * scale;
    scale = pr / sqrt(nhidden+1.0);
    for (int i=0; i<nho+nhidden; ++i) geneweights[nih+i] += NRandom() * scale;
    scale = pr / sqrt(noutput+1.0);
    for (int i=0; i<noutput; ++i) geneweights[nih+nho+nhidden+i] += NRandom() * scale;

    for (int i=0; i<ninputs; ++i) if (IRandom(0,99)<mutationPercentProba) initseq[i] = IRandom(0,nsymbols-1);
}


int MultiLayerPerceptron::estimate(const Vec1& inputs, bool& positive) {
    if (inputs.size()<ninputs) {
        Vec1 completed = inputs;
        for (int i=0; i<ninputs-inputs.size(); ++i) completed.push_back(initseq[i]);
        return estimate(completed, positive);
    }

    // first map symbols to categorical variables
    double din[ninput]; double *inp = &din[0];
    for (int i=0; i<ninputs; ++i) {
        for (int j=0; j<nsymbols; ++j) inp[j] = -1.0;
        inp[inputs[i]] = 1.0; // better than nsymbols number of tests inside previous loop
        inp += nsymbols;
    }

    double dout[noutput];
    computeOutput(din, dout);

    // find back which symbol is selected amongsts non-unknown (there IS a max). If ex-aequo choose as random
    // We could have used softmax for probabilistic interpretation, but:
    // - we're not interested in a full proba dist, just the max value
    // - normalising by softmax doesn't change the max value
    // - using +1/-1 sigmoid doesn't change max either
    // => simply compute max from categorical variables, and don't bother with normalisation
    // Then, choose ex-aequo at random since we may accept non-deterministic outputs in this experiments
    int nchoice = 0;
    int res = 0;
    double bestVote = dout[0]; // start with first non-unknown symbol
    for (int i=1; i<nsymbols; ++i) {
        // a categorical variable has better match?
        if (dout[i]>bestVote) {
            res = i;
            bestVote = dout[i];
            nchoice = 0;
            continue;
        }
        // another categorical variable is ex-aequo?
        if ((dout[i]==bestVote) && (IRandom(0,++nchoice) == 0)) {
            res = i;
        }
    }

    // positive or negative instance?
    positive = (dout[nsymbols] >= 0.0);

    return res;
}

// train is somewhat confusing here. It amounts to taking in account all heard song instances for later batch backprop
void MultiLayerPerceptron::train(const Vec1& inputs, const int output, const bool positive) {

    if (inputs.size()<ninputs) {
        Vec1 completed = inputs;
        for (int i=0; i<ninputs-inputs.size(); ++i) completed.push_back(initseq[i]);
        train(completed, output, positive);
        return;
    }

    // first map I/O symbols to categorical variables -- see the "estimate" function
    Vec1d din(ninput); double *inp = &din[0];
    for (int i=0; i<ninputs; ++i) {
        for (int j=0; j<nsymbols; ++j) inp[j] = -1.0;
        inp[inputs[i]] = 1.0;
        inp += nsymbols;
    }
    assert(output>=0); assert(output<nsymbols);
    Vec1d dout(noutput);
    for (int j=0; j<nsymbols; ++j) dout[j] = -1.0;
    dout[output] = 1.0;
    dout[nsymbols] = positive ? 1.0 : -1.0;

    trainingInstances.push_back( pair<Vec1d, Vec1d>(din, dout) );
}


// Real training is done now, once we have all instances for batch backprop & scaled conjugate gradient
void MultiLayerPerceptron::prepareForEstimation() {

    double dout[noutput];
    double gradout[noutput];

    assert(nsymbols+1==noutput);
    assert(!trainingInstances.empty());

#ifdef MLP_LOG_BPSTEPS
cout << "bpstep errors:";
#endif

    for (int bpstep = 0; bpstep < nbpsteps; ++bpstep) {

#ifdef MLP_LOG_BPSTEPS
        double error = 0.0;
#endif

        for (int t=0; t<trainingInstances.size(); ++t) {

            const Vec1d& inputs = trainingInstances[t].first;
            const Vec1d& targets = trainingInstances[t].second;

            computeOutput(&inputs[0], dout);

            for (int j=0; j<noutput; ++j) gradout[j] = dout[j] - targets[j];
            if (t==0) backPropagate(&inputs[0], dout, gradout);
            else batchBackPropagateAccumulate(&inputs[0], dout, gradout);

#ifdef MLP_LOG_BPSTEPS
            for (int j=0; j<noutput; ++j) error += square(dout[j] - targets[j]);
#endif
        }

        batchBackPropagateTerminate(trainingInstances.size());

#ifdef MLP_LOG_BPSTEPS
        error /= trainingInstances.size()*2; // average on N mappings, of half sum of square
        cout << " " << error;
#endif
        learn(bprate);

    }

#ifdef MLP_LOG_BPSTEPS
cout << endl;
#endif


}


///--------------


// f(x) = x / (1 + abs(x))
static double defaultTransfer(double x) {
    return x / (1.0 + fabs(x));
}

// f'(x) = (1 - abs(f)) ^2
static double defaultTransferDerivativeAsF(double f) {
    return square(1.0 - fabs(f));
}


double (*MultiLayerPerceptron::transfer)(double) = &defaultTransfer;
double (*MultiLayerPerceptron::transferDerivativeAsF)(double) = &defaultTransferDerivativeAsF;


/*
// f(x) = tanh
inline double transfer(const double x) {
    return tanh(x);
}

// f'(x) = 1 - f^2
inline double transferDerivativeAsF(const double f) {
    return 1.0 - f*f;
}
*/

/*
// f(x) = 1/(1+exp(-x))
inline double transfer(const double x) {
    return 1.0/(1.0+exp(-x));
}

// f'(x) = f*(1-f)
inline double transferDerivativeAsF(const double f) {
    return f*(1.0-f);
}
*/

/*
// f(x) = x / (1 + abs(x))
inline double transfer(const double x) {
    return 1.5*x / (1.0 + 1.5*fabs(x));
}

// f'(x) = (1 - abs(f)) ^2
inline double transferDerivativeAsF(const double f) {
    return 1.5 * square(1.0 - fabs(f));
}
*/

void MultiLayerPerceptron::computeOutput( const double * input, double * output )
{
    // compute hidden unit values
    double *rp = hv;  // result pointer
    double *wp = ihw; // weights pointer
    const double* dp; // data pointer
    const double* dpend = input + ninput; // end of data pointer
    for (int h=0; h<nhidden; ++h) {
        *rp = 0.0;
        // compute sum of inputs*weights
        for (dp = input; dp<dpend; ++dp) {
            *rp += *(wp++) * (*dp);
        }
        // apply transfer function
        *rp = transfer(*rp + hb[h]);
        ++rp;
    }

    // compute output values
    rp = output;  // result pointer
    wp = how;     // weights pointer
    dpend = hv + nhidden; // end of data pointer
    for (int o=0; o<noutput; ++o) {
        // linear output function with bias
        *rp = ob[o];
        // compute sum of hidden*weights
        for (dp = hv; dp<dpend; ++dp) {
            *rp += *(wp++) * (*dp);
        }
        ++rp;
    }

}

void MultiLayerPerceptron::backPropagate( const double * input, const double * output, const double * gradout )
{

    // Error gradient by weight: dE/dw = dE/do * do/dw = gradout * do/dw
    // Error gradient by bias: dE/db = gradout * do/db
    // o = b + sum(hidden*weights)
    // So: do/db = 1
    //     do/dw = hidden


    // backpropagation in hidden-to-output layer:
    // dE/dw = gradout * do/dw = gradout * hidden
    // dE/db = gradout * do/db = gradout
    // Thus: howg[h,o] = gradout[o] * hv[h]
    //       obg[o] = gradout[o]

    // backpropagation in input-to-hidden layer:
    // First we need to compute equivalent of gradout for hidden layer
    // with h = value of hidden unit, chain rule on all outputs which depend on h (all of them in this case)
    // dE/dh = sum(dE/do * do/dh), with o = transfer(b + sum(hidden*weights))
    // dE/dh = sum(gradout * weight(h,o) * transferDerivative(bo + sum(h*weight(h,o))))
    // dE/dh = sum(gradout(o) * weight(h,o) * transferDerivativeAsF(o))
    // dE/dh = sum(weight(h,o) * obj[o])
    // Let's store temporarily hbg[h] = sum(how[h,o] * obj[o])
    // Then:
    // dE/dw = dE/dh * dh/dw = dE/dh * input_value * transferDerivativeAsF(h)
    // dE/db = dE/dh * dh/db = dE/dh * transferDerivativeAsF(h)
    // Thus, combining dE/dh with the above formula:
    //     hbg[h]_final = hbg[h]_temp * transferDerivativeAsF(hv[h])
    //     ihwg[i,h] = hbg[h]_final * input[i]

    // step one, compute all hidden-to-output related variables: obg[o], howg[h,o], and temporary dE/dh in hbg[h]
    const double* dp; // hidden value data pointer
    const double* dpend = hv + nhidden; // const because it will be reused for input data
    double* bgp = obg;  // bias gradient pointer: dE/db for b output bias
    double* wgp = howg; // weights gradient pointer: dE/dw for w hidden-to-output weight
    double* tgp;        // gradout equivalent for hidden layer: dE/dh for h hidden value
    double* tgpend = hbg + nhidden;
    double* wp = how;   // weights pointer: for dE/dh computation
    for (tgp = hbg; tgp<tgpend; ++tgp) *tgp = 0.0; // initialize sum for dE/dh 
    for (int o=0; o<noutput; ++o) {
        *bgp = gradout[o]; // see above
        for (dp = hv; dp<dpend; ++dp) {
            *(wgp++) = *bgp * (*dp);       // see above: howg[h,o] = obg[o] * hv[h]
        }
        for (tgp = hbg; tgp<tgpend; ++tgp) {
            *tgp += *(wp++) * (*bgp);      // see above: hbg[h] += how[h,o] * obg[o]
        }
        ++bgp;
    }

    // step two: compute input-to-hidden variables: ihwg[i,h] and final hbg[h]
    dpend = input + ninput;
    bgp = hbg;  // bias gradient pointer: dE/db for b hidden bias
    wgp = ihwg; // weights gradient pointer: dE/dw for w input-to-hidden weight
    for (int h=0; h<nhidden; ++h) {
        *bgp *= transferDerivativeAsF(hv[h]); // hbg[h]_final = hbg[h]_temp * transferDerivativeAsF(hv[h])
        for (dp = input; dp<dpend; ++dp) {
            *(wgp++) = *bgp * (*dp);          // see above: ihwg[i,h] = hbg[h]_final * input[i]
        }
        ++bgp;
    }

    // Done!!!
}


void MultiLayerPerceptron::batchBackPropagateAccumulate( const double * input, const double * output, const double * gradout )
{
    // See comments in the previous function
    // The only difference here is that gradients are accumulated over previous
    // ones, instead of replacing them
    
    const double* dp; // hidden value data pointer
    const double* dpend = hv + nhidden; // const because it will be reused for input data
    double* bgp = obg;    // bias gradient pointer: dE/db for b output bias
    double* wgp = howg;   // weights gradient pointer: dE/dw for w hidden-to-output weight
    double temp[nhidden]; // will be used to store the temporary sum(how[h,o] * obj[o]), we can't override hbg here
    double* tgp;          // gradout equivalent for hidden layer: dE/dh for h hidden value
    double* tgpend = temp + nhidden;
    double* wp = how;     // weights pointer: for dE/dh computation
    double current_bgp;   // another temporary value for a variable we can't erase
    
    for (tgp = temp; tgp<tgpend; ++tgp) *tgp = 0.0; // initialize sum for dE/dh

    for (int o=0; o<noutput; ++o) {
        current_bgp = gradout[o]; // see above
        for (dp = hv; dp<dpend; ++dp) {
            *(wgp++) += current_bgp * (*dp);    // see above: howg[h,o] = obg[o] * hv[h]. Accumulate result
        }
        for (tgp = temp; tgp<tgpend; ++tgp) {
            *tgp += *(wp++) * current_bgp;      // see above: hbg[h]_temp += how[h,o] * obg[o]
        }
        *(bgp++) += current_bgp;                // Accumulate result
    }

    // step two: compute input-to-hidden variables: ihwg[i,h] and final hbg[h]
    dpend = input + ninput;
    bgp = hbg;  // bias gradient pointer: dE/db for b hidden bias
    wgp = ihwg; // weights gradient pointer: dE/dw for w input-to-hidden weight
    for (int h=0; h<nhidden; ++h) {
        // hbg[h]_final = hbg[h]_temp * transferDerivativeAsF(hv[h]).
        current_bgp = temp[h] * transferDerivativeAsF(hv[h]); 
        for (dp = input; dp<dpend; ++dp) {
            *(wgp++) += current_bgp * (*dp);    // see above: ihwg[i,h] = hbg[h]_final * input[i]. Accumulate result
        }
        *(bgp++) += current_bgp;                // Accumulate result
    }
}

void MultiLayerPerceptron::batchBackPropagateTerminate(int nmappings)
{
    double factor = 1.0 / (double)nmappings;
    for (int i=0; i<nih; ++i) ihwg[i] *= factor;
    for (int i=0; i<nho; ++i) howg[i] *= factor;
    for (int i=0; i<nhidden; ++i) hbg[i] *= factor;
    for (int i=0; i<noutput; ++i) obg[i] *= factor;
}

void MultiLayerPerceptron::learn( double learningRate )
{
    for (int i=0; i<nih; ++i) ihw[i] -= ihwg[i] * learningRate;
    for (int i=0; i<nho; ++i) how[i] -= howg[i] * learningRate;
    for (int i=0; i<nhidden; ++i) hb[i] -= hbg[i] * learningRate;
    for (int i=0; i<noutput; ++i) ob[i] -= obg[i] * learningRate;
}

double MultiLayerPerceptron::trainOnce( const double * input, const double * target, double learningRate )
{
    double output[noutput];
    computeOutput(input, output);
    
    // simple error function: half sum of square => gradient is difference
    double gradout[noutput];
    for (int j=0; j<noutput; ++j) gradout[j] = output[j] - target[j];
    
    // backpropagation of error gradient
    backPropagate(input, output, gradout);
    learn(learningRate);
    
    // compute error
    double error = 0.0;
    for (int j=0; j<noutput; ++j) error += square(output[j] - target[j]);
    return error * 0.5; // half sum of square
}

void MultiLayerPerceptron::getHidden( double * hidden )
{
    for (int i=0; i<nhidden; ++i) hidden[i] = hv[i];
}

void MultiLayerPerceptron::setHidden( const double * hidden )
{
    for (int i=0; i<nhidden; ++i) hv[i] = hidden[i];
}

std::ostream& operator<<(std::ostream& os, const MultiLayerPerceptron& mlp){
    
    os << mlp.ninput << " " << mlp.nhidden << " " << mlp.noutput << endl;
    
    for (int i=0; i<mlp.nih; ++i) os << mlp.ihw[i] << " ";
    os << endl;
    
    for (int i=0; i<mlp.nhidden; ++i) os << mlp.hb[i] << " ";
    os << endl;
    
    for (int i=0; i<mlp.nho; ++i) os << mlp.how[i] << " ";
    os << endl;

    for (int i=0; i<mlp.noutput; ++i) os << mlp.ob[i] << " ";
    os << endl;

    return os;
}

void MultiLayerPerceptron::printPrefixed(string prefix) {
    cout << prefix << "I/H/O: " << ninput << " " << nhidden << " " << noutput << endl;
    cout << prefix << "I/H weights: ";
    for (int i=0; i<nih; ++i) cout << ihw[i] << " ";
    cout << endl;
    cout << prefix << "I/H biases: ";
    for (int i=0; i<nhidden; ++i) cout << hb[i] << " ";
    cout << endl;
    cout << prefix << "H/O weights: ";
    for (int i=0; i<nho; ++i) cout << how[i] << " ";
    cout << endl;
    cout << prefix << "H/O biases: ";
    for (int i=0; i<noutput; ++i) cout << ob[i] << " ";
    cout << endl;
}

std::istream& operator>>(std::istream& is, MultiLayerPerceptron& mlp) {
    
    is >> mlp.ninput;
    is >> mlp.nhidden;
    is >> mlp.noutput;
    mlp.nih = mlp.ninput * mlp.nhidden;
    mlp.nho = mlp.nhidden * mlp.noutput;
    
    delete [] mlp.ihw; delete [] mlp.how; delete [] mlp.ihwg;
    delete [] mlp.howg; delete [] mlp.hb; delete [] mlp.ob;
    delete [] mlp.hbg; delete [] mlp.obg; delete [] mlp.hv;
    
    mlp.ihw = new double[mlp.nih]; mlp.ihwg = new double[mlp.nih];
    for (int i=0; i<mlp.nih; ++i) {
        is >> mlp.ihw[i];
        mlp.ihwg[i] = 0.0;
    }

    mlp.hb = new double[mlp.nhidden]; mlp.hbg = new double[mlp.nhidden]; mlp.hv = new double[mlp.nhidden];
    for (int i=0; i<mlp.nhidden; ++i) {
        is >> mlp.hb[i];
        mlp.hbg[i] = 0.0;
        mlp.hv[i] = 0.0;
    }

    mlp.how = new double[mlp.nho]; mlp.howg = new double[mlp.nho];
    for (int i=0; i<mlp.nho; ++i) {
        is >> mlp.how[i];
        mlp.howg[i] = 0.0;
    }

    mlp.ob = new double[mlp.noutput]; mlp.obg = new double[mlp.noutput];
    for (int i=0; i<mlp.noutput; ++i) {
        is >> mlp.ob[i];
        mlp.obg[i] = 0.0;
    }

    return is;
}

void MultiLayerPerceptron::mutate( double ihwRate, double ihwJitter, double howRate, double howJitter, double hbRate, double hbJitter, double obRate, double obJitter )
{
    double rangeRate = ihwRate * 2.0;
    double offsetRate = 1.0 - ihwRate;
    double offsetJitter = - ihwJitter / sqrt(ninput+1.0);
    double rangeJitter = - 2.0 * offsetJitter;
    for (int i=0; i<nih; ++i) {
        ihw[i] *= offsetRate + rangeRate * Random();
        ihw[i] += offsetJitter + rangeJitter * Random();
    }

    rangeRate = howRate * 2.0;
    offsetRate = 1.0 - howRate;
    offsetJitter = - howJitter / sqrt(nhidden+1.0);
    rangeJitter = - 2.0 * offsetJitter;
    for (int i=0; i<nho; ++i) {
        how[i] *= offsetRate + rangeRate * Random();
        how[i] += offsetJitter + rangeJitter * Random();
    }
    
    rangeRate = hbRate * 2.0;
    offsetRate = 1.0 - hbRate;
    offsetJitter *= hbJitter / howJitter; //= - hbJitter / sqrt(nhidden+1.0);
    rangeJitter = - 2.0 * offsetJitter;
    for (int i=0; i<nhidden; ++i) {
        hb[i] *= offsetRate + rangeRate * Random();
        hb[i] += offsetJitter + rangeJitter * Random();
    }
    
    rangeRate = obRate * 2.0;
    offsetRate = 1.0 - obRate;
    offsetJitter = - obJitter / sqrt(noutput+1.0);
    rangeJitter = - 2.0 * offsetJitter;
    for (int i=0; i<noutput; ++i) {
        ob[i] *= offsetRate + rangeRate * Random();
        ob[i] += offsetJitter + rangeJitter * Random();
    }
    
}

//=========================================
// DEBUG CODE BELOW
//=========================================

#if 1 // def MULTILAYERPERCEPTRON_H_DEBUG_MODE

#include <iostream>
using namespace std;


// Test routines

void MultiLayerPerceptron::gradCheck()
{

    double* input = new double[ninput];
    for (int i=0; i<ninput; ++i) input[i] = NRandom();
    double* target = new double[noutput];
    for (int i=0; i<noutput; ++i) target[i] = NRandom();
    double* output = new double[noutput];

    computeOutput(input, output); // random garbage
    
    // simple error function: half sum of square => gradient is difference
    double* gradout = new double[noutput];
    for (int i=0; i<noutput; ++i) gradout[i] = output[i] - target[i];
    
    // test our backpropagation
    backPropagate(input, output, gradout);

    // compute explicitly the gradients
    int nih = ninput * nhidden;
    int nho = nhidden * noutput;
    double epsilon = 1.0e-5;
    double sav;
    double eplus, eminus;
    // ih weights derivative
    for (int i=0; i<nih; ++i) {
        sav = ihw[i];
        ihw[i] += epsilon;
        computeOutput(input, output);
        eplus = 0.0;
        for (int j=0; j<noutput; ++j) eplus += square(output[j] - target[j]);
        eplus *= 0.5; // half sum of square
        ihw[i] = sav - epsilon;
        computeOutput(input, output);
        eminus = 0.0;
        for (int j=0; j<noutput; ++j) eminus += square(output[j] - target[j]);
        eminus *= 0.5; // half sum of square
        ihw[i] = sav;
        // Use central difference formula for approximation of error gradient
        // difference between explicit and backprop values
        cout << "diff for ihwg[" << i << "] = " << 0.5*(eplus - eminus)/epsilon - ihwg[i] << endl;
    }
    for (int i=0; i<nho; ++i) {
        sav = how[i];
        how[i] += epsilon;
        computeOutput(input, output);
        eplus = 0.0;
        for (int j=0; j<noutput; ++j) eplus += square(output[j] - target[j]);
        eplus *= 0.5; // half sum of square
        how[i] = sav - epsilon;
        computeOutput(input, output);
        eminus = 0.0;
        for (int j=0; j<noutput; ++j) eminus += square(output[j] - target[j]);
        eminus *= 0.5; // half sum of square
        how[i] = sav;
        // Use central difference formula for approximation of error gradient
        // difference between explicit and backprop values
        cout << "diff for howg[" << i << "] = " << 0.5*(eplus - eminus)/epsilon - howg[i] << endl;
    }
    for (int i=0; i<nhidden; ++i) {
        sav = hb[i];
        hb[i] += epsilon;
        computeOutput(input, output);
        eplus = 0.0;
        for (int j=0; j<noutput; ++j) eplus += square(output[j] - target[j]);
        eplus *= 0.5; // half sum of square
        hb[i] = sav - epsilon;
        computeOutput(input, output);
        eminus = 0.0;
        for (int j=0; j<noutput; ++j) eminus += square(output[j] - target[j]);
        eminus *= 0.5; // half sum of square
        hb[i] = sav;
        // Use central difference formula for approximation of error gradient
        // difference between explicit and backprop values
        cout << "diff for hbg[" << i << "] = " << 0.5*(eplus - eminus)/epsilon - hbg[i] << endl;
    }
    for (int i=0; i<noutput; ++i) {
        sav = ob[i];
        ob[i] += epsilon;
        computeOutput(input, output);
        eplus = 0.0;
        for (int j=0; j<noutput; ++j) eplus += square(output[j] - target[j]);
        eplus *= 0.5; // half sum of square
        ob[i] = sav - epsilon;
        computeOutput(input, output);
        eminus = 0.0;
        for (int j=0; j<noutput; ++j) eminus += square(output[j] - target[j]);
        eminus *= 0.5; // half sum of square
        ob[i] = sav;
        // Use central difference formula for approximation of error gradient
        // difference between explicit and backprop values
        cout << "diff for obg[" << i << "] = " << 0.5*(eplus - eminus)/epsilon - obg[i] << endl;
    }
    
}

int main(int argc, char** argv) {

//    MultiLayerPerceptron mlp(7,9,5);

    double input[] = {1,2,3,4,5,6,7,8,9};
    double target[] = {0.2,0.3,0.5,0.7,0.11,0.13};
    const int noutput = 6;
    const int ninput = 9;
    const int nhidden = 20;
    double output[noutput];
    double gradout[noutput];

    MultiLayerPerceptron mlp(ninput,nhidden,noutput);
    mlp.init();
    mlp.randCreateGeneticInfo();

#if 1
    mlp.gradCheck();
    return 0;
#endif

    // training on only one mapping    
    for (int i=0; i<1000; ++i) {
        double error = mlp.trainOnce(input, target, 0.1);
        mlp.computeOutput(input, output);
        cout << "Error at step " << i << " is "<< error << ", outputs are ";
        for (int j=0; j<noutput; ++j) cout << output[j] << " ";
        cout << endl;
    }

    double input2[] = {7,5,9,9,6,4,5,23,1};
    double target2[] = {0.21,-.02,-.36,-.68,0.8,0.4};
    double input3[] = {4,8,9,-5,14,2,3,1,4};
    double target3[] = {-0.5,0.8,0.7,0.1,-0.3,0.2};
    
    MultiLayerPerceptron mlp2(ninput,nhidden,noutput);
    mlp2.init();
    mlp2.randCreateGeneticInfo();
    
    // batch backprop on several mappings
    for (int i=0; i<10000; ++i) {
        double error = 0.0;
        
        // see comments in train()
        mlp2.computeOutput(input, output);
        for (int j=0; j<noutput; ++j) gradout[j] = output[j] - target[j];
        mlp2.backPropagate(input, output, gradout);
        for (int j=0; j<noutput; ++j) error += square(output[j] - target[j]);
        
        // second mapping
        mlp2.computeOutput(input2, output);
        for (int j=0; j<noutput; ++j) gradout[j] = output[j] - target2[j];
        mlp2.batchBackPropagateAccumulate(input, output, gradout);
        for (int j=0; j<noutput; ++j) error += square(output[j] - target2[j]);

        // third mapping
        mlp2.computeOutput(input3, output);
        for (int j=0; j<noutput; ++j) gradout[j] = output[j] - target3[j];
        mlp2.batchBackPropagateAccumulate(input, output, gradout);
        for (int j=0; j<noutput; ++j) error += square(output[j] - target3[j]);

        // terminate batch backpropagation: 3 mappings
        mlp2.batchBackPropagateTerminate(3);

        // Learn for next step using gradients from batch backpropagation
        mlp2.learn(0.1);
        
        // terminate error computation
        error /= 3*2; // average on 3 mappings, of half sum of square

        cout << "Batch backprop error at step " << i << " is "<< error << endl;
    }
    
    return 0;
}

#endif // TWOLAYERPERCEPTRON_H_DEBUG_MODE




