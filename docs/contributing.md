# Contributing to CrashCatch

Thank you for considering contributing to CrashCatch! We welcome contributions that improve the functionality, documentation, or general usability of the project. By following the guidelines below, you'll help ensure that your contributions are aligned with the project's goals and ready for review.

## How to Contribute

We follow a simple process for contributions:

1. **Fork the Repository**:  
   Fork the [CrashCatch repository](https://github.com/keithpotz/CrashCatch) to your own GitHub account.

2. **Clone Your Fork**:  
   Clone your fork to your local machine:
   ```bash
   git clone https://github.com/your-username/CrashCatch.git
   ```

3. **Create a New Branch:**
    Create a new branch for your changes:
    ```bash
    git checkout -b your-branch-name
    ```
4. **Make Your Changes**:
 * Write code or modifyy documentation as needed.
 * Ensure your changes follow the project's coding standards and guidelines.
 * If you are adding a new feature or fixing a bug, please add unit tests or modify exisitn gone where appropriate.
5. **Commit Your Changes**:
    Commit your changes with a clear and concise commit message. Use the following format:
    ```bash
    git add .
    git commit -m "Your commit message"
    ```
6. **Push Your Changes**:
    Push your changes to your fork:
    ```bash
    git push origin your-branch-name
    ```
7. **Create a Pull Request**:
 * Once you've pushed your changes, go to the original repository and create a pull request from your fork. Be sure to:
 * Provide a description of the changes you've made.
 * Reference any relevant issues (e.g., Fixes #123).
 * Ensure that your changes pass any automated tests before submitting.

## Coding Style
We strive for consistency in our codebase. Here are a few key points to keep in mind.
 + Use camelCase for variable and fucntion names
 + Use 4 spaces for indentation, not tabs.
 + Add comments to explain complex logic or any important assumptions in the code.

## Documentation
If you are contributing to the documentation, please ensure that:
 + The text is clear and easy to understand.
 + You follow the format used in other documentation files. 
 + You verify that all links and code samples are correct.

## Testing
We rely on automated tests to ensure the quality of the coddebase. If you are submitting a bug fix or a new feature:
  + Please add or update tests to cover your changes.
  + Ensure that all existing tests pass by running:
    
    ```bash
    #For C++;
    make test
    ```
## Issues 
If you encounter any issues while using CrashCatch, please open an issue on the [GitHub repository](https://github.com/keithpotz/CrashCatch/issues). Provide as much information as possible about the issue, including the steps to reproduce it and any relevant error messages.

## Code of conduct
We respect the rights of all contributors and follow the [Contributor Covenant](https://www.contributor-covenant.org/) to ensure a positive and respectful community.

## License
CrashCatch is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
