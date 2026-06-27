# PTY Backend

Polo's active Unix terminal backend is `src/core/PtySession.*`.

It uses `forkpty` to start an interactive shell per tab, sends keyboard input to the PTY master, and streams output back into the terminal view.

When `/bin/bash` exists, Polo starts:

```text
bash --rcfile assets/polo_bashrc -i
```

The rc file defines:

- `say`, Polo's print command
- red invalid-command output
- `> ` prompt
- OSC 7 current-directory updates for the top path label

Windows needs a separate ConPTY implementation. That should live behind the same `PtySession` interface so the UI does not need platform-specific branching.
