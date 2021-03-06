## CSV Data files description

### Calibration and fine-tune 

First of all, here's the description of each parameter:
- *T* is the sample period, i.e. the minimum delay between samples.
- The *step* is how much the output varies between levels.
- *Samples* is the amount of samples **per output level!**
- *A* is the value of the step in 8bit pwm signal (0 to 255 range).

#### d) Voltage in ldr as a function of led pwm signal

| File          | Command                               | Parameters                          |
|---------------|---------------------------------------|-------------------------------------|
|`fig_d_1.csv`  | `sweep(0, 255, 1, 100, 1, VOLT)`      | T = 100 ms step = 1  samples = 1    |
|`fig_d_2.csv`  | `sweep(0, 255, 1, 100, 1, VOLT)`      | T = 100 ms step = 10 samples = 1    |
|`fig_d_3.csv`  | `sweep(0, 255, 10, 0.2, 500, VOLT)`   | T = 0.2 ms step = 10 samples = 500  |

#### e) Steady-state voltage in ldr raw ADC input as a function of led pwm signal

| File          | Command                               | Parameters                          |
|---------------|---------------------------------------|-------------------------------------|
|`fig_e.csv`    | `sweep(0, 255, 10, 50, 1, RAW)`       | T = 50 ms  step = 1                 |

#### f) Previous figures in LUX units

| File          | Command                               | Parameters                          |
|---------------|---------------------------------------|-------------------------------------|
|`fig_f_1.csv`  | `sweep(0, 255, 10, 0.2, 500, LUX)`    | T = 0.2 ms step = 10 samples = 500  |
|`fig_f_2.csv`  | `sweep(0, 255, 10, 50, 1, LUX)`       | T = 50 ms  step = 1                 |

#### g) Step response for calculating time constant

| File          | Command                               | Parameters                          |
|---------------|---------------------------------------|-------------------------------------|
|`fig_g_1.csv`  | `sweep(0, 128, 128, 0.5, 300, VOLT)`  | T = 0.5 ms A = 128 samples = 300    |
|`fig_g_2.csv`  | `sweep(0, 128, 128, 0.5, 300, LUX)`   | T = 0.5 ms A = 128 samples = 300    |

The time constant corresponds to the time interval between the start of the step and the output
reaching 63% (-3 dB) of the y(end).
The static gain, G(0) can be obtained from the approximated transfer function of the plant.

**If we use the step response in Volt we obtain τ = 11.2 ± 0.5 ms and G(0) = 0.02442.**

The response in LUX has a lot of oscillation, which makes it a poor candidate to use for parameter
estimation.
The numerical method in MATLAB to estimate the linearized system fails after 20 iterations
while achieving little to no progress in minimizing the cost function.

### Controller features

#### h) Feedforward comparison

| File            | Descritption                        |
|-----------------|-------------------------------------|
|`fig_h_0.csv`    | Feedforward off                     |
|`fig_h_0_5.csv`  | Feedforward on, K_FF = 0.5          |
|`fig_h_1.csv`    | Feedforward on, K_FF = 1.0          |

#### i) FF + PI reference delay comparison

| File            | Descritption                        |
|-----------------|-------------------------------------|
|`fig_i_1.csv`    | 1 Sample delay                      |
|`fig_i_2.csv`    | 2 Samples delay                     |
|`fig_i_3.csv`    | 3 Samples delay                     |

#### j) Anti-windup comparison

| File            | Descritption                        |
|-----------------|-------------------------------------|
|`fig_j_1.csv`    | Anti-windup off                     |
|`fig_j_2.csv`    | Simple Anti-windup                  |
|`fig_j_3.csv`    | Feedback Anti-windup K_SAT = 0.5    |

#### k) Error deadzone comparison

| File            | Descritption                        |
|-----------------|-------------------------------------|
|`fig_k_1.csv`    | Error deadzone off                  |
|`fig_k_2.csv`    | Error deadzone [-0.5,0.5]           |
|`fig_k_3.csv`    | Error deadzone [-1.6,1.6]           |
