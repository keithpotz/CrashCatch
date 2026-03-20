# Contributing to CrashCatch

Thank you for considering contributing to CrashCatch! We welcome contributions that improve the functionality, documentation, or general usability of the project. By following the guidelines below, you'll help ensure that your contributions are aligned with the project's goals and ready for review.

## How to Contribute

We follow a simple process for contributions:

1. **Fork the Repository**
   Fork the [CrashCatch repository](https://github.com/keithpotz/CrashCatch) to your own GitHub account.

2. **Clone Your Fork**
   Clone your fork to your local machine:
   ```bash
   git clone https://github.com/your-username/CrashCatch.git
   ```

3. **Create a New Branch**
   Create a new branch for your changes:
   ```bash
   git checkout -b your-branch-name
   ```

4. **Make Your Changes**
   - Write code or modify documentation as needed.
   - Ensure your changes follow the project's coding standards and guidelines.
   - If you are adding a new feature or fixing a bug, add or update tests in `tests/` where appropriate.

5. **Commit Your Changes**
   Commit your changes with a clear and concise message:
   ```bash
   git add .
   git commit -m "Brief description of change"
   ```

6. **Push Your Changes**
   ```bash
   git push origin your-branch-name
   ```

7. **Create a Pull Request**
   - Go to the original repository and open a pull request from your fork.
   - Provide a clear description of the changes you've made.
   - Reference any relevant issues (e.g., `Fixes #123`).
   - Ensure your changes build cleanly before submitting.

---

## Coding Style

- Use `camelCase` for variable and function names
- Use 4 spaces for indentation, not tabs
- Add comments to explain complex logic or important assumptions
- Keep platform-specific code behind `#ifdef CRASHCATCH_PLATFORM_WINDOWS` / `#ifdef CRASHCATCH_PLATFORM_LINUX`

---

## Testing

CrashCatch has a `tests/` directory with CMake-integrated test targets. If you are submitting a bug fix or new feature:

- Add or update tests to cover your changes
- Build and run the tests before submitting:

```bash
cmake --build build --config Debug
cd build/Debug
./test_callback_order.exe
./test_stack_context.exe
```

---

## Documentation

If you are contributing to the documentation:

- Follow the format and style used in existing `.md` files in `docs/`
- Verify all links and code samples are correct
- Keep language clear and concise

---

## Issues

If you encounter any issues while using CrashCatch, please open an issue on the [GitHub repository](https://github.com/keithpotz/CrashCatch/issues). Include steps to reproduce, platform information, and any relevant error messages.

---

## Code of Conduct

We respect the rights of all contributors and follow the [Contributor Covenant](https://www.contributor-covenant.org/) to ensure a positive and respectful community.

---

## License

CrashCatch is licensed under the MIT License. See the [LICENSE](license.md) file for details.
