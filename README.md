# Logic Solver

<div align="center">
  <img src="Pictures/ESI_Logo.png" alt="Logic Solver Logo" width="200">
  <h3>Advanced Propositional Logic Solver with Resolution Refutation</h3>
  <p>Developed at École nationale Supérieure d'Informatique (ESI)</p>
  

  
</div>

## 📖 Overview

Logic Solver is a powerful tool designed for analyzing and verifying propositional logic formulas in Conjunctive Normal Form (CNF) using the resolution by refutation method introduced by J.A. Robinson in 1965. This project combines a robust resolution engine with an intuitive graphical user interface to provide both computational power and ease of use.

<div align="center">
  <img src="Pictures/schreenshot.png" alt="Logic Solver Screenshot" width="600">
</div>

## ✨ Features

- 📝 **Create and manage logical formulas** in CNF format
- 🔍 **Test formulas for satisfiability** using resolution by refutation
- 📊 **View and organize your formula collection**
- 👁️ **Preview formulas** with proper logical notation (∧, ∨, ¬)
- 🧠 **Advanced resolution algorithm** with unit propagation and tautology detection
- 🔄 **CNF conversion module** to transform arbitrary formulas into CNF

## 🚀 Getting Started

### Prerequisites

- Windows operating system
- No additional dependencies required

### Installation

1. Download the latest release from the [Releases](https://github.com/yourusername/logic-solver/releases) page
2. Extract the ZIP file to your preferred location
3. Ensure both `gui_solver.exe` and `logic_solver.exe` are in the same directory
4. Run `gui_solver.exe` to start the application
5. Use the input_generator.exe to run the program without the interface in case the gui doesn't work for you

## 📚 How to Use

### Creating a New Formula

1. Select "Create New Formula" from the main menu
2. Enter a name for your formula (without .cnf extension)
3. Specify the number of clauses you want to enter
4. For each clause:
   - Enter literals separated by spaces
   - Use ! for negation (e.g., !P for NOT P)
   - Example: `P !Q R` represents (P OR NOT Q OR R)
5. The formula will be automatically saved in CNF format

### Testing a Formula

1. Select "Test Existing Formula" from the main menu
2. Choose a formula from the list
3. View the result (SATISFIABLE or UNSATISFIABLE) in the popup window

### Viewing Available Formulas

1. Select "Show Available Formulas" from the main menu
2. Browse the list of saved formulas
3. Select a formula to view its contents with proper logical notation
4. Optionally, test the selected formula directly from this interface

### CNF File Structure

Formulas are stored in plain text files with the .cnf extension:

```
P !Q R
!P S
T !U
```

This represents the formula: (P ∨ ¬Q ∨ R) ∧ (¬P ∨ S) ∧ (T ∨ ¬U)

- Each line represents a clause (connected by AND)
- Within each line, literals are separated by spaces (connected by OR)
- Negation is represented by ! before the literal

## 🧠 Technical Details

### Resolution by Refutation Algorithm

The core of Logic Solver is based on Robinson's resolution by refutation algorithm:

1. Read the CNF formula from file
2. Apply resolution steps iteratively:
   - Find pairs of clauses containing complementary literals (P and !P)
   - Generate resolvent clauses by combining and simplifying these pairs
   - Add new resolvents to the clause set
3. If the empty clause is derived, the formula is UNSATISFIABLE
4. If no more resolution steps can be applied without deriving the empty clause, the formula is SATISFIABLE

### Optimizations

- **Unit Clause Propagation:** Prioritize resolution with unit clauses
- **Tautology Elimination:** Remove clauses containing both P and !P
- **Subsumption:** Remove clauses that are supersets of other clauses
- **Pure Literal Elimination:** Simplify formulas with literals appearing with only one polarity

## 🌟 Examples

### Example 1: Simple Contradiction

File: `contradiction.cnf`
```
P
!P
```
Result: `UNSATISFIABLE`

### Example 2: Valid Formula

File: `valid.cnf`
```
P Q
P !Q
!P Q
!P !Q
```
Result: `SATISFIABLE`

### Example 3: Graph Coloring Problem

Representing a triangle graph with 2 colors:
File: `graph_coloring.cnf`
```
P_11 P_12
P_21 P_22
P_31 P_32
!P_11 !P_21
!P_11 !P_31
!P_21 !P_31
!P_12 !P_22
!P_12 !P_32
!P_22 !P_32
```
Result: `UNSATISFIABLE` (A triangle cannot be colored with 2 colors)




## 👥 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository

2. Open a Pull Request



## 👨‍💻 Authors

- **LASFER Mohammed Djawed**
- **MOUSLIM Ali**
- **HECHEHOUCHE Aboubakeur**

Developed as part of the Advanced Logic course at École nationale Supérieure d'Informatique (ESI), 2024-2025.

## 🙏 Acknowledgments

- **Meziani Lila** - Project Supervisor
- **Khelifati Si Larbi** - Academic Advisor
- **Faical Touka** - Academic Advisor
- **Charabi Leila** - Academic Advisor
- J.A. Robinson for the original resolution algorithm
