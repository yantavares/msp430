#### **Exercise 1: Pin Configuration**

Write a program where the green LED mimics the state of button S1:

- **Pressed S1 → LED on.**
- **Released S1 → LED off.**

---

#### **Exercise 2: Button Debouncing (without debounce)**

Create a routine in the `main` function that toggles the red LED every time button S1 is pressed. Do not implement debounce logic.

---

#### **Exercise 3: Button Debouncing (with debounce)**

Redo **Exercise 2**, but remove button bounce (debounce). Implement a `debounce()` function that introduces a delay via a loop decrementing a volatile variable. Ensure the compiler does not optimize it out.

---

#### **Exercise 4: Dual Button Action**

Modify the program to toggle the red LED state whenever **either button S1 or S2** transitions from open to closed. Specifically:

- **Action occurs when the state changes from open (A) to closed (F).**

---

#### **Exercise 6: Sampling Timer Flags**

Write a program to blink the green LED (P4.7) at exactly 1 Hz:

- **0.5 seconds on and 0.5 seconds off.**
- Use a polling approach to check the overflow (TAIFG) or channel 0 (CCIFG) flags of the timer.

---

#### **Problem 2:**

Implement a system to control a servo motor (e.g., SG90) using PWM:

- Configure the timer to output a PWM signal with a **50 Hz period (20 ms)**.
- Adjust the pulse width for servo positioning:
  - 0.5 ms (minimum angle).
  - 2.5 ms (maximum angle).
- Use buttons S1 and S2 to vary the pulse width in steps of **0.1 ms**.
- Ensure:
  - **Brown wire → GND.**
  - **Red wire → +5V.**
  - **Orange wire → PWM signal.**
