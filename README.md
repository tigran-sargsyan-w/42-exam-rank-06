# 42 – Exam Rank 06 Practice 🌐

> Practice implementation of **42 Exam Rank 06** in C (`mini_serv`), a small TCP chat server built with `select()`.
> Personal study repository, **not for cheating during the exam**.

---

## ⚠️ Disclaimer

This repository is **only for personal study and revision**:

- It is **not** an official 42 resource.
- It is **not** intended to be used during the actual exam.
- The exercise subject and supplied starter code were not written by me.
- Exam subjects and evaluation requirements may change. Always follow the subject provided during your exam.
- Please respect your campus rules and the spirit of the exam.

Use this project to understand socket programming, practice `select()`, and learn to build a simple multi-client server from scratch.

---

## 📂 Repository Structure

```text
42-exam-rank-06/
├── mini_serv.c       # Practice solution
├── main.c            # Starter/reference code
├── subject.en.txt    # Exercise subject
└── README.md
```

> Only `mini_serv.c` is listed as an expected submission file in the subject. The provided `main.c` is for reference and may contain functions or behavior that are not allowed in the final submission.

---

## 🧩 Exercise Overview

### `mini_serv` — TCP Chat Server

Build a small server that allows multiple clients to exchange text messages over TCP.

The server:

- Takes a **port number** as its command-line argument.
- Listens **only on `127.0.0.1`** (localhost).
- Accepts multiple clients and assigns sequential IDs starting from `0`.
- Announces when clients connect or disconnect.
- Forwards each received line to **every other connected client**, prefixing it with the sender's ID.
- Uses `select()` to handle multiple sockets without letting a slow client block the entire server.

### Message format

When a client connects, the clients already connected receive:

```text
server: client 1 just arrived
```

When client `1` sends `Hello!`, the other clients receive:

```text
client 1: Hello!
```

When that client disconnects, the remaining clients receive:

```text
server: client 1 just left
```

Each line must have its own `client <id>: ` prefix, even if a single `recv()` call contains multiple newline characters.

---

## ⚙️ Important Subject Requirements

- **No `#define` directives** are allowed.
- Use only the functions permitted by the subject.
- Handle clients with `select()`; do not assume one `recv()` equals one complete line.
- Do not disconnect a client just because it is slow to read incoming messages.
- Do not introduce unnecessary buffering or delays: the tester expects messages to be forwarded promptly.
- Avoid memory leaks and file-descriptor leaks.
- The final executable must use the exact required error messages on `stderr` and exit with status `1` when applicable:

| Situation | Required output |
| --- | --- |
| Wrong number of arguments | `Wrong number of arguments\n` |
| Fatal system-call error before accepting connections | `Fatal error\n` |
| Memory allocation failure | `Fatal error\n` |

> The subject notes that socket operations may block if `recv()` or `send()` is called without first checking readiness through `select()`. Do not rely on checking `EAGAIN` as a workaround.

---

## 🛠️ Build & Run

From the repository root:

```bash
# Compile the server
cc -Wall -Wextra -Werror mini_serv.c -o mini_serv

# Start listening on port 8080
./mini_serv 8080
```

The server binds to `127.0.0.1:8080` and waits for connections. Choose another available port if `8080` is already in use.

### Connect clients with Netcat

Open **two or more additional terminals** while the server is running.

Terminal 2 — client 0:

```bash
nc 127.0.0.1 8080
```

Terminal 3 — client 1:

```bash
nc 127.0.0.1 8080
```

Type messages into either client terminal and press **Enter**. The other client should receive lines such as:

```text
client 0: Hello from client 0!
client 1: Hello from client 1!
```

These are example lines seen by the *other* client, not a transcript from one terminal: messages should not be echoed back to their sender by the server.

---

## 🧪 Manual Testing Checklist

There is no custom automated test suite included in this repository. I use `nc` and multiple terminals to verify the main behaviors:

- [ ] Start the server with a valid port.
- [ ] Run the executable with no arguments and confirm the exact error message and exit status.
- [ ] Connect several clients and check IDs start at `0` and increase.
- [ ] Check arrival notifications reach the clients already connected.
- [ ] Send messages in both directions; the sender must not receive its own broadcast.
- [ ] Send several newline-separated messages in one input.
- [ ] Split a message across multiple sends and check that lines are handled correctly.
- [ ] Disconnect a client and check the departure notification.
- [ ] Connect another client and verify IDs are not reused.
- [ ] Verify a slow-reading client does not prevent the server from serving others.
- [ ] Check for memory and file-descriptor leaks during local testing.

> These checks are for practice only. They do not replace the official 42 evaluation and cannot guarantee coverage of every edge case.

---

## 🧠 Key Concepts to Understand

**1. TCP sockets:** Creating a listening socket, binding it to an address and port, listening, and accepting connections.

**2. `select()` and `fd_set`:** Monitoring file descriptors for read and write readiness so that one inactive client does not stop the rest of the server.

**3. Client state:** Maintaining the mapping between socket file descriptors, client IDs, and any incomplete received messages.

**4. TCP is a byte stream:** A read may contain part of a line, one line, or several lines. Message boundaries must be reconstructed using `\n`.

**5. Broadcasting:** Sending messages and connection events to the correct clients while handling potential partial writes and slow readers.

**6. Resource management:** Closing disconnected sockets, freeing allocated memory, and handling errors consistently.

---

## 🗺️ Mental Map — `6 → 6 → 2 → 6 → 1`

To rebuild `mini_serv` from memory, I use a simple numerical mnemonic:

> **MINI_SERV: 6 INCLUDES → 6 VARIABLE GROUPS → 2 STARTER HELPERS → 6 FUNCTIONS → 1 MAIN**

This is a **memorization aid for my practice implementation**, not a required structure from the official subject. The goal is to remember the order of the building blocks, then understand and write the logic behind each one. Explicitly including `<sys/select.h>` helps declare `fd_set` and `select()` without relying on another header to include it indirectly.

```text
MINI_SERV: 6 - 6 - 2 - 6 - 1
│
├── 6 INCLUDES
│   ├── <unistd.h>
│   ├── <string.h>
│   ├── <stdlib.h>
│   ├── <stdio.h>
│   ├── <netinet/ip.h>
│   └── <sys/select.h>
│
├── 6 VARIABLE GROUPS
│   ├── count
│   ├── max_fd
│   ├── ids
│   ├── msgs
│   ├── fd_sets: rfds / wfds / afds
│   └── buffers: buf_read / buf_write
│
├── 2 STARTER HELPERS
│   ├── extract_message
│   └── str_join
│
├── 6 FUNCTIONS (in writing order)
│   ├── fatal_error
│   ├── notify_other
│   ├── register_client
│   ├── remove_client
│   ├── send_msg
│   └── create_socket
│
└── 1 MAIN
    ├── SETUP (6 steps): argc → zero → socket → address → bind → listen
    └── LOOP (6 steps):  while → reset → select → for → accept / recv
```

### The `main()` flow at a glance

```text
START
  |
  v
check argc → initialize state → socket → 127.0.0.1 → bind → listen
  |
  v
while (1)
  |
  +-- reset read/write fd sets from the active set
  |
  +-- select() → iterate over ready fds
                    |
                    +-- listening socket → accept()
                    |                      → assign next client ID
                    |                      → broadcast "just arrived"
                    |
                    +-- client socket → recv()
                                           |
                                           +-- <= 0 → disconnect
                                           |          → broadcast "just left"
                                           |
                                           +-- > 0  → accumulate input
                                                      → extract complete lines
                                                      → add "client ID: " prefix
                                                      → broadcast to others
```

**How I practice with this map:** first recite **`6–6–2–6–1`**, then write the function names in order, sketch the setup and event loop, and finally implement each block without looking at the solution. For the two helpers, review the **starter code supplied with the exercise**; do not assume it is unchanged in every exam version.

---

## 📖 How I Use This Repo

- Read the subject carefully before writing code.
- Rebuild `mini_serv` **from scratch** without copy-paste.
- Practice the socket setup and `select()` loop separately.
- Test with several `nc` clients, including unusual message boundaries.
- Respect the exact allowed-functions list and required output format.
- Repeat until I can explain every part of the server and implement it under exam conditions.

If you are preparing for **42 Exam Rank 06**, use this repository as a reference and practice ground. The goal is not to memorize a working solution, but to understand how a multi-client TCP server works well enough to write one yourself.
