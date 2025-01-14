#### **Exercise 11: `reduceSum16`**

Write a subroutine to compute the sum of all 16-bit integers in a vector and return a 32-bit result. Use the `ADDC` instruction to handle carry-over between the least significant word (LSWord) and most significant word (MSWord). The subroutine should:

- **Input:**
  - `R12` → Address of the vector.
  - `R13` → Size of the vector (number of 16-bit elements).
- **Output:**
  - `R12` → LSWord of the sum.
  - `R13` → MSWord of the sum.

Test the subroutine with a vector initialized using `.word`.

---

#### **Exercise 19: `W16_ASC`**

Write a subroutine to convert a 16-bit unsigned number into its ASCII hexadecimal representation and store it in memory. Use an auxiliary subroutine, `NIB_ASC`, to convert each nibble (4 bits) into its corresponding ASCII character. The subroutine should:

- **Input:**
  - `R12` → 16-bit unsigned number to convert.
  - `R13` → Address of the output vector.
- **Output:**
  - At `R13`, store the ASCII characters representing the hexadecimal nibbles of `R12`.
  - Example: If `R12 = 0x89AB`, the memory at `R13` should contain the bytes corresponding to `'8'`, `'9'`, `'A'`, and `'B'` (ASCII values 0x38, 0x39, 0x41, 0x42).
