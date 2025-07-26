# Mine++

**Mine++** is a C++ remake of my very first game developed with Unity: a Minesweeper.
This remake is a personal challenge in spatial optimization — aiming to use as little memory as possible during execution, without sacrificing performance.

🔗 [Original Unity version](https://github.com/lvocanson/Minesweeper)

---

## Build

This project uses **CMake** for cross-platform building.  
Refer to the [CMake documentation](https://cmake.org/documentation/) if you're not familiar with how to use it.

### Quickstart (Windows + Visual Studio)

1. Open a **Command Prompt** in your desired directory.
2. Run the following command:
   ```cmd
   git clone https://github.com/lvocanson/MinePlusPlus.git ./MinePlusPlus && start devenv ./MinePlusPlus
   ```
3. Visual Studio will open the project.
4. In the **Solution Explorer**, open `CMakeLists.txt`, then press `Ctrl + S` to trigger CMake generation.  
   You can track the progress in the **Output Window**, under the **CMake** category.
5. Once generation completes, set the correct startup item at the top of the screen (next to the green play button) by selecting `MinePlusPlus.exe`.
