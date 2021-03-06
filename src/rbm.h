#ifndef restricted_boltzmann_machine_H 
#define restricted_boltzmann_machine_H
#include "matrix_functions.h"
#include "random.h"

typedef struct {
  // Basic description of a restricted Boltzman machine.
  int n_inputs;           // Number of nodes in the lower 'input' layer.
  int n_outputs;          // Number of nodes in the upper 'output' layer.
  
  matrix_t *io_weights;   // n_inputs x n_outputs matrix representing weights between input and output nodes.
  double *bias_inputs;    // 1x n_inputs vector representing the 'bias' toward activation of the input nodes.
  double *bias_outputs;   // 1x n_outputs vector representing the 'bias' toward activation of the input nodes.

  // Basic learning parameters.
  double learning_rate;   // Rate at which the model learns.
  int cd_n;               // Specifies the number of Gibbs sampling steps used for contrastive divergence during training.
  int batch_size;         // Specifies the batch size for training.
  int update_input_bias;  // Specifies whether input biases are updated during learning, or fixed (e.g. from a previous layer).
  
  // Special learning options.  These are NOT guaranteed to be set.
  // See in: http://www.cs.utoronto.ca/~ilya/pubs/ilya_sutskever_phd_thesis.pdf; pp. 75; also see: pp.5(background),73(Adapting Nesterov methods).
  int use_momentum;       // Use momentum during fitting.
  double momentum_decay;  // \Mu; Rate at which old gradients are discarded.
  matrix_t *momentum;     // Momentum term; serves as memory for other mini-batch members.  Speeds the rate of convergence.
  double *input_momentum; // Momentum term for input biases.
  double *output_momentum;// Momentum term for output biases.

  int use_l2_penalty;     // Usees L2 regularization to constrain weight updates.
  double weight_cost;     // Alpha term in the L2 regularization function.

  // Also implement sparsity controls(?!).
} rbm_t;

typedef struct {
  // Abstraction used to store the graident at a point in both RMBs and DBNs.
  matrix_t *delta_w;
  double *delta_output_bias;
  
  int input_bias_allocated; // Logical; determines whether or not to update input biases.  Prevents input_biases from being updated by a dbn during backpropagation.
  double *delta_input_bias;
  
  // Applying delta_w uses the parameters specified here.
  int batch_size;
} delta_w_t;

#define rbm_sample_state(prob) ((prob*10000)>(xorshift()%10000)?1:0)
#define logistic_function(value) (1/ (1+exp(-value)))


void clamp_input(rbm_t *rbm, double *input, double *resulting_output);
void clamp_output(rbm_t *rbm, double *output, double *resulting_input);

rbm_t *alloc_rbm(int n_inputs, int n_outputs);
void free_rbm(rbm_t *rbm);
void init_rbm(rbm_t *rbm, double learning_rate, int batch_size, int cd_n, double expected_frequency_on);

void apply_delta_w(rbm_t *rbm, delta_w_t *dw);
void initial_momentum_step(rbm_t *rbm);
void apply_momentum_correction(rbm_t *rbm, delta_w_t *dw);

void free_delta_w_ptr(delta_w_t *dw, int n);
void rbm_train(rbm_t *rbm, double *input_example, int n_examples, int n_epocs, int n_threads);

delta_w_t *alloc_dwt_from_rbm(rbm_t *rbm);

rbm_t *rbm_r_to_c(SEXP rbm_r);
rbm_t *rbm_layer_r_to_c(SEXP rbm_r, double *points_to_bias_inputs);

SEXP train_rbm_R(SEXP rbm_r, SEXP training_data_r, SEXP n_epocs_r, SEXP n_threads_r);

// MUTEX for training...
#ifdef _POSIX_THREADS 
#include <pthread.h>
pthread_mutex_t rbm_mutex;
#endif

#endif
