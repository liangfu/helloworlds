#ifndef MULTILAYERPERCEPTRON_H
#define MULTILAYERPERCEPTRON_H

#include <iostream>
// #include "DataDef.h"
#include <vector>

 int ninputs, nsymbols;
 int nhiddens, nbpsteps;
 double bprate;

// class Vec1{};
// class Vec1d{public:Vec1d(){}Vec1d(int){}void resize(int){}int size(){}};

static void dummpy(){srand(time(NULL));}

typedef std::vector<double> Vec1d;
typedef std::vector<int> Vec1;
float Random(){return static_cast<float>(rand())/static_cast<float>(RAND_MAX);}
int BRandom(){return rand()%2;}
int IRandom(int begin,int end){assert(end>begin);return rand()%(end-begin);}
double NRandom() {
  double x;
  do {
    x = Random();
  } while (x==0.0);
  x = sqrt(-2.0 * log(x));
  if (Random() > 0.5) return x * cos((2.0*3.14159265358979323846) * Random());
  return x * sin((2.0*3.14159265358979323846)* Random());
}

class MultiLayerPerceptron {
public:

// Individual API ---------------

    // Initial creation of the individual with random values - replaces a load from archive
    void randCreateGeneticInfo();
    // setup the internal structures after a load or random create
    void setupFromGeneticInfo() {}

    void train(const Vec1& inputs, const int output, const bool positive);
    int estimate(const Vec1& inputs, bool& positive);

    // Genetic functions
    void crossover(const MultiLayerPerceptron& parent1, const MultiLayerPerceptron& parent2);
    void mutate(int mutationPercentProba = 3);
    // clone genetic info
    void clone(const MultiLayerPerceptron& parent);

    // non-genetic info may be flushed and re-init to save memory
    void init();
    // flushs memory.
    void unload();

    void prepareForEstimation();

  void printPrefixed(std::string prefix="");

    // more genetic info
    Vec1 initseq;

// ------------------------------



    MultiLayerPerceptron(int nin, int nhid, int nout);
    MultiLayerPerceptron();
    MultiLayerPerceptron(const MultiLayerPerceptron& mlp);
    MultiLayerPerceptron& operator=(const MultiLayerPerceptron& mlp) {throw "Unimplemented TODO if used";}
    ~MultiLayerPerceptron();

    /** Set a transfer function. 

        Default is a custom transfer function:
            f(x) = x / (1 + abs(x))
        This function is sigmoid-like, -1 / +1 bounded, continuous to any order, and much faster
        to compute than tanh. It is also unfortunately slower to converge, but about the same
        speed as the sigmoid function for this project according to preliminary experiments.
    */
    static double (*transfer)(double);
    
    /** Set the derivative of the transfer function, expressed in terms of the original function.
        This is the differential equation relating f' and f. Such an equation does not always exist,
        but when it does, it provides a big boost for the backpropagation. In practice in neural
        networks, only such functions are thus used. Sorry, but this class does not handle the more
        generic case.

        Default is a custom transfer function I devised especially for this project:
            f' = (1 - abs(f)) ^2         f'(x) = 1 / (1 + abs(x))^2
    */
    static double (*transferDerivativeAsF)(double);

    /** Gets the output corresponding to this input vector */
    void computeOutput(const double* input, double* output);

    /** Backpropagates the error function gradients in the network
        This function pre-supposes the current internal values of the hidden units match
        the given input to output mapping. This is the case is computeOutput was called
        previously to this function. This is usually necessary anyway to compute the error
        gradient, so it isn't a big requirement.
        @param input The training data requested input. Array of size ninput
        @param output The training data desired output. Array of size noutput
        @param gradout The gradient of the error function on each of the outputs. Array of size noutput
        @return the error
    */
    void backPropagate(const double* input, const double* output, const double* gradout);

    /** Batch backpropagation accumulates all gradients from all mappings one by one.
        Use backPropagate on the first mapping, then call this function to accumulate
        the gradients for all other mappings. In the end, call batchBackPropagateTerminate
        with the total number of mappings.
        This function pre-supposes the current internal values of the hidden units match
        the given input to output mapping. This is the case is computeOutput was called
        previously to this function. This is usually necessary anyway to compute the error
        gradient, so it isn't a big requirement.
        @param input The training data requested input. Array of size ninput
        @param output The training data desired output. Array of size noutput
        @param gradout The gradient of the error function on each of the outputs. Array of size noutput
        @return the error
    */
    void batchBackPropagateAccumulate(const double* input, const double* output, const double* gradout);

    /** Batch backpropagation accumulates all gradients from all mappings one by one.
        Use backPropagate on the first mapping, then call batchBackPropagateAccumulate
        to accumulate the gradients for all other mappings. In the end, call this function
        with the total number of mappings.
        This function pre-supposes the current internal values of the hidden units match
        the given input to output mapping. This is the case is computeOutput was called
        previously to this function. This is usually necessary anyway to compute the error
        gradient, so it isn't a big requirement.
        @param input The training data requested input. Array of size ninput
        @param output The training data desired output. Array of size noutput
        @param gradout The gradient of the error function on each of the outputs. Array of size noutput
        @return the error
    */
    void batchBackPropagateTerminate(int nmappings);
    

    /// Default learning rate for the training by gradient descent. Default is 0.1.
    static const double defaultLearningRate;
    
    /** Very simple gradient descent, by the given amount.
        Uses the current gradients to update the weights and bias.
        @param learningRate The amount of descent to go along the gradient.
    */
    void learn(double learningRate = defaultLearningRate);

    /** Commodity function to train the network for the given target using
        the very common 'half sum of square of the output' error.
        You may call this function repeatedly to train the network, checking
        the results till you're satisfied
        @param learningRate The amount of descent to go along the gradient.
        @return the current error.
    */
    double trainOnce(const double *input, const double *target, double learningRate = defaultLearningRate);

    /** Read-write accessors to hidden values allow to store the results of computeOutput
        for later backpropagation
    */
    void getHidden(double* hidden);
    
    /** Read-write accessors to hidden values allow to store the results of computeOutput
        for later backpropagation
    */
    void setHidden(const double* hidden);

    /// Read-only accessors
    inline int getNInput() {return ninput;}
    inline int getNOutput() {return noutput;}
    inline int getNHidden() {return nhidden;}

    /** Mutate this network weights and biases with the given parameters.
        This has nothing to do in this generic class, but I'm too lazy to split it cleanly.
        
        For each "ihw" input-to-hidden weight, each "how" hidden-to-output weight
        each "hb" hidden bias, each "ob" output bias, the following parameters apply.
        @param Rate The network weights are added a random number between -mutationRate*currentValue
            and +mutationRate*currentValue. 0 thus makes a perfect copy, which may be useful
            at the beginning to build a large population from only a few teachers.
        @param Jitter The network weights are also added a random number between
            -jitter value & +jitter value, normalized by the network layer dimensions.
            This allows to give a chance to null weights, which would otherwise not be 
            affected by the mutation rate.
    */
    void mutate(double ihwRate, double ihwJitter, double howRate, double howJitter, double hbRate, double hbJitter, double obRate, double obJitter);


protected:
// data members

    int ninput, nhidden, noutput;
    int nih; // = ninput * nhidden;
    int nho; // = nhidden * noutput;

    // Aliases
    double *ihw; // input to hidden weights: array of size ninput * nhidden
    double *how; // hidden to output weights: array of size nhidden * noutput

    double *ihwg; // input to hidden weight gradients: array of size ninput * nhidden
    double *howg; // hidden to output weight gradients: array of size nhidden * noutput

    double *hb;  // hidden unit bias: array of size nhidden
    double *ob;  // output unit bias: array of size output

    double *hbg;  // hidden unit bias gradients: array of size nhidden
    double *obg;  // output unit bias gradients: array of size output

    double *hv;  // hidden values holder

    // True memory arrays
    Vec1d weights;
    Vec1d gradients;
    Vec1d geneweights;
    Vec1d hiddenValues;

    // Training data set for scaled conjugate gradients: map inputs to output symbol
  std::vector< std::pair<Vec1d, Vec1d> > trainingInstances;

public:

// #ifdef GEPNET_ARCHIVE
//     template<class Archive> void save(Archive & ar, const unsigned int version) const {
//         if (archivingGeneticInfo) {
//             for (int i=0; i<ninputs; ++i) ar & initseq[i];
//             for (int i=0; i<geneweights.size(); ++i) {double v = geneweights[i]; ar & v;}
//         }
//         assert(!weights.empty());
//         for (int i=0; i<weights.size(); ++i) {double v = weights[i]; ar & v;}
//         for (int i=0; i<gradients.size(); ++i) {double v = gradients[i]; ar & v;}
//         for (int i=0; i<hiddenValues.size(); ++i) {double v = hiddenValues[i]; ar & v;}
//     }
//     template<class Archive> void load(Archive & ar, const unsigned int version) {
//         if (archivingGeneticInfo) {
//             initseq.resize(ninputs);
//             for (int i=0; i<ninputs; ++i) ar & initseq[i];
//             for (int i=0; i<geneweights.size(); ++i) {double v; ar & v; geneweights[i] = v;}
//         }
//         init();
//         for (int i=0; i<weights.size(); ++i) {double v; ar & v; weights[i] = v;}
//         for (int i=0; i<gradients.size(); ++i) {double v; ar & v; gradients[i] = v;}
//         for (int i=0; i<hiddenValues.size(); ++i) {double v; ar & v; hiddenValues[i] = v;}
//     }
//     BOOST_SERIALIZATION_SPLIT_MEMBER()
// #endif

friend std::ostream& operator<<(std::ostream& os, const MultiLayerPerceptron& tlp);
friend std::istream& operator>>(std::istream& is, MultiLayerPerceptron& tlp);

// DEBUG Code: see cpp file
  void gradCheck();
// #ifdef MULTILAYERPERCEPTRON_H_DEBUG_MODE
// MULTILAYERPERCEPTRON_H_DEBUG_MODE
// #endif
};

std::ostream& operator<<(std::ostream& os, const MultiLayerPerceptron& tlp);
std::istream& operator>>(std::istream& is, MultiLayerPerceptron& tlp);

#endif
