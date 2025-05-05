Develop a Tic-tac-toe (OXO) Game using Process Management on RISC-V

**Objective:**  
Create a multi-process implementation of Tic-tac-toe with three processes: a referee (parent) and two players (children).

**Technical Requirements:**
1. Operating System: RISC-V
2. Process Structure:
   - Referee (parent process): Manages game state, display, and turn coordination
   - Player A (child process): Makes moves using 'X'
   - Player B (child process): Makes moves using 'O'

**Game Board Representation:**
```
C1 C2 C3
C4 C5 C6
C7 C8 C9
```
- Empty cell: 9
- X marker: -1
- O marker: 1

**Game Logic:**
1. Initial board sum = 81 (9 empty cells × 9)
2. Game ends when either:
   - Win condition: Any row/column/diagonal sums to ±3
   - Draw condition: |sum of all cells| = 1

**Implementation Requirements:**
1. Use pipes for inter-process communication
2. Implement random but valid move selection for players
3. Add 1-second delay between moves
4. Include UML documentation for:
   - Game initialization
   - Player A's turn
   - Player B's turn
   - Win scenario
   - Game termination

**Required Functions:**
1. `abs()` for absolute value calculation
2. Board state validation
3. Win condition checking
4. Display functions
5. Process communication handlers

**Expected Output Format:**
```
JEU DU MORPION:
_ _ _
_ _ _
_ _ _
```

**Command Interface:**
```bash
$ oxo
```

**Documentation Requirements:**
- UML diagrams for each game phase
- Pipe communication flow documentation
- Function specifications

Notes: Follow previous project structure guidelines and carefully manage parent-child process data sharing.