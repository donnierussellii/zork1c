//****************************************************************************
//The following is derived from Geoff Kuenning's implementation
//of the Mersenne Twist pseudorandom number generator:
//http://www.cs.hmc.edu/~geoff/mtwist.html


#define DEFAULT_SEED32_OLD  4357

#define MT_STATE_SIZE  624

#define RECURRENCE_OFFSET  397

#define UPPER_MASK  0x80000000
#define LOWER_MASK  0x7fffffff
#define COMBINE_BITS(x,y)  (((x)&UPPER_MASK)|((y)&LOWER_MASK))

#define MATRIX_A  0x9908b0df
#define MATRIX_MULTIPLY(original,new)  ((original)^((new)>>1)^matrix_decider[(new)&0x1])

#define KNUTH_MULTIPLIER_OLD  69069
#define KNUTH_MULTIPLIER_NEW  1812433253ul
#define KNUTH_SHIFT 30

#define MT_TEMPERING_SHIFT_U(y)  (y>>11)
#define MT_TEMPERING_SHIFT_S(y)  (y<<7)
#define MT_TEMPERING_SHIFT_T(y)  (y<<15)
#define MT_TEMPERING_SHIFT_L(y)  (y>>18)
#define MT_TEMPERING_MASK_B      0x9d2c5680
#define MT_TEMPERING_MASK_C      0xefc60000


typedef struct
{
  unsigned int statevec[MT_STATE_SIZE];
  int stateptr;
  int initialized;
} mt_state;


mt_state mt_default_state;

unsigned int matrix_decider[2] = {0x0, MATRIX_A};


void mts_refresh(mt_state *state);
void mts_seed32(mt_state *state, unsigned int seed);
void mts_seed32new(mt_state *state, unsigned int seed);


void mts_refresh(mt_state *state)
{
  int i;
  unsigned int *state_ptr;
  unsigned int value1;
  unsigned int value2;

  if (!state->initialized)
  {
    mts_seed32(state, DEFAULT_SEED32_OLD);
    return;
  }

  state_ptr = &state->statevec[MT_STATE_SIZE-1];
  value1 = *state_ptr;
  for (i=(MT_STATE_SIZE-RECURRENCE_OFFSET)/2; --i>=0; )
  {
    state_ptr -= 2;
    value2 = state_ptr[1];
    value1 = COMBINE_BITS(value1, value2);
    state_ptr[2] = MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET+2], value1);
    value1 = state_ptr[0];
    value2 = COMBINE_BITS(value2, value1);
    state_ptr[1] = MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET+1], value2);
  }
  value2 = *--state_ptr;
  value1 = COMBINE_BITS(value1, value2);
  state_ptr[1] = MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET+1], value1);

  for (i=(RECURRENCE_OFFSET-1)/2; --i>=0; )
  {
    state_ptr -= 2;
    value1 = state_ptr[1];
    value2 = COMBINE_BITS(value2, value1);
    state_ptr[2] = MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE-RECURRENCE_OFFSET+2], value2);
    value2 = state_ptr[0];
    value1 = COMBINE_BITS(value1, value2);
    state_ptr[1] = MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE-RECURRENCE_OFFSET+1], value1);
  }

  value1 = COMBINE_BITS(value2, state->statevec[MT_STATE_SIZE-1]);
  *state_ptr = MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE-RECURRENCE_OFFSET], value1);

  state->stateptr = MT_STATE_SIZE;
}



void mts_seed32(mt_state *state, unsigned int seed)
{
  int i;

  if (seed==0) seed = DEFAULT_SEED32_OLD;

  state->statevec[MT_STATE_SIZE-1] = seed&0xffffffff;

  for (i=MT_STATE_SIZE-2; i>=0; i--)
    state->statevec[i] = (KNUTH_MULTIPLIER_OLD*state->statevec[i+1])&0xffffffff;

  state->stateptr = MT_STATE_SIZE;
  state->initialized = 1;

  mts_refresh(state);
}



void mts_seed32new(mt_state *state, unsigned int seed)
{
  int i;
  unsigned int nextval;

  state->statevec[MT_STATE_SIZE-1] = seed&0xffffffffUL;
  for (i=MT_STATE_SIZE-2; i>=0; i--)
  {
    nextval = state->statevec[i+1]>>KNUTH_SHIFT;
    nextval ^= state->statevec[i+1];
    nextval *= KNUTH_MULTIPLIER_NEW;
    nextval += (MT_STATE_SIZE-1)-i;
    state->statevec[i] = nextval&0xffffffffUL;
  }

  state->stateptr = MT_STATE_SIZE;
  state->initialized = 1;

  mts_refresh(state);
}



void mt_seed32new(unsigned int seed)
{
  mts_seed32new(&mt_default_state, seed);
}



unsigned int mt_lrand(void)
{
  unsigned int random_value;

  if (mt_default_state.stateptr<=0) mts_refresh(&mt_default_state);

  random_value = mt_default_state.statevec[--mt_default_state.stateptr];

  random_value ^= MT_TEMPERING_SHIFT_U(random_value);
  random_value ^= MT_TEMPERING_SHIFT_S(random_value)&MT_TEMPERING_MASK_B;
  random_value ^= MT_TEMPERING_SHIFT_T(random_value)&MT_TEMPERING_MASK_C;

  return random_value^MT_TEMPERING_SHIFT_L(random_value);
}
//****************************************************************************



//****************************************************************************
void InitRandom(unsigned int seed)
{
  mt_seed32new(seed);
}



int GetRandom(int range)
{
  if (range < 1) return 0;

  return ((mt_lrand() & 0x0000FFFFU) * range) >> 16;
}
//****************************************************************************
