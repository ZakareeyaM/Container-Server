# Container Management Client–Server Application

A desktop container and pallet management system built with **C++17** and the **Qt framework**. The project is split into two independently built applications:

- **Container** – the client application used to create Box and Cylinder containers, organise them into pallets, back up unallocated containers, and send pallet data as XML.
- **ContainerServer** – the server application that listens for the client's TCP connection, receives the XML document, validates container codes, and displays the received records in a table.

The two applications communicate over **TCP on port `6164`**. In the current source code, the client connects to `127.0.0.1`, so the supplied configuration expects both applications to run on the **same computer**.

---

## Contents

- [Features](#features)
- [Architecture](#architecture)
- [Project structure](#project-structure)
- [Technology stack](#technology-stack)
- [Data model](#data-model)
- [Container codes](#container-codes)
- [Client-to-server communication](#client-to-server-communication)
- [XML message format](#xml-message-format)
- [Backup and restore](#backup-and-restore)
- [Build requirements](#build-requirements)
- [Build and run](#build-and-run)
- [How to use](#how-to-use)
- [Important implementation notes](#important-implementation-notes)
- [Possible future improvements](#possible-future-improvements)

---

## Features

### Container client

- Create **Box** containers using length, breadth, height, and weight.
- Create **Cylinder** containers using diameter, height, and weight.
- Generate a unique-format code for each newly created container.
- Display containers that have not yet been assigned to a pallet.
- Move a selected container to a pallet number.
- Calculate container volume and pallet totals.
- Generate and display an XML representation of the pallet data.
- Send the generated XML to the server over TCP.
- Back up and restore the list of unallocated containers using `backup.txt`.
- Provide Help, About, Backup, Restore, Post XML, and Exit menu actions.

### Container server

- Start and stop listening for client connections.
- Listen on TCP port `6164` on the available network interfaces (`QHostAddress::Any`).
- Receive XML data from the client.
- Parse Box and Cylinder elements from the XML document.
- Check container codes against the expected code pattern.
- Display received data in a table containing pallet number, container type, code, dimensions, and weight.
- Report connection, XML parsing, and data-reception errors through the application interface and debug output.

---

## Architecture

The application follows a small **client–server architecture**. The client owns the container and pallet data and is responsible for preparing the XML message. The server receives that message and presents the parsed information.

```text
+----------------------------------+          TCP / XML           +----------------------------------+
|          Container Client        |  --------------------------> |          Container Server       |
|                                  |       127.0.0.1:6164          |                                  |
|  Qt Widgets user interface       |                               |  Qt Widgets user interface       |
|                                  |                               |                                  |
|  ContainerWidget                 |                               |  ContainerServer                 |
|    |                             |                               |    |                             |
|    +-- ContainerFactory           |                               |    +-- QTcpServer                |
|    +-- Container / Box / Cylinder |                               |    +-- QTcpSocket                |
|    +-- Pallet                     |                               |    +-- QXmlStreamReader           |
|    +-- Serializing                |                               |    +-- QStandardItemModel         |
|                                  |                               |                                  |
|  QTcpSocket                      |                               |  Table view                      |
+----------------------------------+                               +----------------------------------+
```

### Client-side responsibilities

1. The user enters dimensions and weight in the Qt Widgets interface.
2. `ContainerFactory` creates the selected concrete container type and assigns its code.
3. New containers are held in the unallocated-container collection.
4. The user selects a container and assigns it to a `Pallet`.
5. `Serializing` traverses the pallet collection and writes the XML document using `QXmlStreamWriter` and the Qt meta-object properties exposed by `Box` and `Cylinder`.
6. XML generation runs through a worker object moved to a `QThread`.
7. The client displays the generated XML and uses `QTcpSocket` to connect to the server and send the UTF-8 XML data.

### Server-side responsibilities

1. The server window is opened and the user selects **Start Listening**.
2. `QTcpServer` listens on port `6164`.
3. When a client connects, the server obtains its `QTcpSocket` and handles its `readyRead`, `disconnected`, and socket-error signals.
4. The received bytes are converted from UTF-8 to a string.
5. The server checks that the message appears to begin with an XML declaration or the `<pallets>` root element.
6. `QXmlStreamReader` parses the document. Each completed Box or Cylinder element is converted into a row in the table model.
7. The server reports whether parsing succeeded and how many container records were displayed.

### High-level component relationships

| Component | Responsibility |
|---|---|
| `MainClient` | Main client window, menus, application actions, Help and About dialogs. |
| `ContainerWidget` | Client interface and coordination of container creation, pallet assignment, backup/restore, XML generation and sending. |
| `Container` | Abstract base class containing common code and weight data. |
| `Box` | Box dimensions and box volume calculation. |
| `Cylinder` | Cylinder dimensions and cylinder volume calculation. |
| `ContainerFactory` | Creates Box or Cylinder objects and generates container codes. |
| `Pallet` | Holds containers assigned to a pallet and calculates total weight and volume. |
| `Serializing` | Converts pallet/container data into an XML document. |
| `ContainerServer` | Server window, TCP listener/client socket handling, XML parsing and table display. |

---

## Project structure

```text
Container-Server App/
│
├── README.md
│
├── Container/                       # Client application
│   ├── CMakeLists.txt
│   ├── main.cpp                     # Client application entry point
│   ├── mainclient.h / .cpp          # Main window and menus
│   ├── containerwidget.h / .cpp     # Main container-management interface
│   ├── container.h / .cpp           # Container, Box and Cylinder classes
│   ├── containerfactory.h / .cpp    # Container creation and code generation
│   ├── pallet.h / .cpp              # Pallet data and totals
│   ├── serializing.h / .cpp         # XML generation worker
│   ├── Icons.qrc                    # Client Qt resources
│   └── *.png, *.ico                 # Client icons and splash artwork
│
└── ContainerServer/                 # Server application
    ├── CMakeLists.txt
    ├── main.cpp                     # Server application entry point
    ├── containerserver.h / .cpp     # TCP server, XML parser and table interface
    ├── Icons.qrc                    # Server Qt resources
    └── server_icon.ico
```

The client and server each have their own `CMakeLists.txt` and build into separate executables. They are not configured as one combined CMake target in the supplied project.

---

## Technology stack

| Technology | Use in this project |
|---|---|
| C++17 | Application logic and object-oriented data model. |
| Qt Widgets | Desktop graphical user interfaces. |
| Qt Network | TCP client/server communication (`QTcpSocket` and `QTcpServer`). |
| Qt XML | XML writing and parsing (`QXmlStreamWriter` and `QXmlStreamReader`). |
| Qt Core | Containers, strings, dates, signals/slots, threading and other core facilities. |
| CMake 3.16+ | Build configuration for each application. |

The client CMake configuration requests **Qt Widgets, Network and Xml**. The server requests **Qt Widgets and Network**. The project CMake files support finding Qt 6 or Qt 5; use a Qt installation that includes the requested modules and a compatible C++17 compiler.

---

## Data model

### Container hierarchy

`Container` is an abstract base class. It stores the common container code and weight and defines the interface implemented by concrete container types.

```text
                  Container (abstract)
                  /                  \
                 /                    \
              Box                    Cylinder
```

The base class defines the common operations:

- `getType()` – returns the container type name.
- `getVolume()` – returns the calculated volume.
- `clone()` – creates a copy of the concrete container.
- `getCode()` / `setCode()` – reads or changes the generated code.
- `getWeight()` / `setWeight()` – reads or changes the weight.

`Box` adds `length`, `breadth`, and `height`. Its volume calculation is:

```text
volume = length × breadth × height
```

`Cylinder` adds `diameter` and `height`. Its volume calculation uses the radius (`diameter / 2`) and rounds the result to the nearest integer:

```text
volume = round(π × radius² × height)
```

Both concrete classes use `Q_GADGET` and `Q_PROPERTY` so their properties can be read by the XML serializer through Qt's meta-object system.

### Pallet

A `Pallet` has a pallet number and a collection of `Container` pointers. It provides operations to add and remove containers and to calculate:

- **Total weight** – the sum of the weights of its containers.
- **Total volume** – the sum of the calculated volumes of its containers.

The pallet destructor deletes the containers it owns. The client stores pallets in a `QMap<int, Pallet*>`, keyed by pallet number.

---

## Container codes

The factory generates codes in this general format:

```text
YYYY/MM/B<number>   Box
YYYY/MM/C<number>   Cylinder
```

For example, a code has the structure `2025/08/B1` or `2025/08/C1` (the actual year and month are taken from the system date when the container is created).

Box and Cylinder serial counters are maintained separately while the client is running. The server checks codes against this regular-expression pattern:

```regex
^\d{4}/\d{2}/[BC]\d+$
```

A code that does not match is displayed as `****` in the server table. This is a format check; it is not a check that the code is globally unique or that the date is a valid calendar month.

---

## Client-to-server communication

| Setting | Current implementation |
|---|---|
| Protocol | TCP |
| Server port | `6164` |
| Client destination | `127.0.0.1` |
| Payload encoding | UTF-8 |
| Payload format | XML |
| Connection handling | Client connects, writes the XML, then disconnects. |

**Important:** the client currently calls `connectToHost("127.0.0.1", 6164)`. This is the loopback address, meaning it connects to a server on the same computer. Although the server listens on `QHostAddress::Any`, a client on another computer cannot reach it using the current client setting. To use separate computers, the client destination needs to be changed to the server computer's LAN address (and the server computer's firewall must allow inbound TCP port `6164`).

The server implementation maintains one active `clientSocket` pointer and replaces/cleans up an existing client connection when a new connection is accepted. It should therefore be treated as a simple single-active-client receiver, not a multi-client session manager.

---

## XML message format

The client creates an XML document with a `<pallets>` root element. The root includes a `NumberOfPallets` attribute. Each `<pallet>` has `number`, `weight`, and `volume` attributes. A pallet contains `<Box>` and/or `<Cylinder>` elements.

Illustrative structure (values are examples):

```xml
<?xml version="1.0"?>
<pallets NumberOfPallets="1">
    <pallet weight="25" volume="6000" number="1">
        <Box>
            <code>2025/08/B1</code>
            <weight>10</weight>
            <length>20</length>
            <breadth>15</breadth>
            <height>10</height>
        </Box>
        <Cylinder>
            <code>2025/08/C1</code>
            <weight>15</weight>
            <diameter>10</diameter>
            <height>20</height>
        </Cylinder>
    </pallet>
</pallets>
```

The example illustrates the structure written by the serializer; the actual values are generated from the user's pallet data. The server reads the container type, code, dimensions and weight and places them in these table columns:

1. Pallet
2. Container
3. Code
4. Height
5. Breadth/Diameter
6. Length
7. Weight

For a Cylinder, the Length column is left blank. Pallet-level weight and volume are included as XML attributes, but the server's displayed table is organised around individual container records.

---

## Backup and restore

The client includes a plain-text backup mechanism for **unallocated containers**. It writes to a file named:

```text
backup.txt
```

The file is read from or written to the application's current working directory. Its format is pipe-delimited, with comment lines and a container count:

```text
# Container Backup File
# Format: TYPE|CODE|WEIGHT|PARAM1|PARAM2|PARAM3
# Box: TYPE|CODE|WEIGHT|LENGTH|BREADTH|HEIGHT
# Cylinder: TYPE|CODE|WEIGHT|DIAMETER|HEIGHT
CONTAINER_COUNT=2

BOX|2025/08/B1|10|20|15|10
CYLINDER|2025/08/C1|15|10|20
```

- **Backup Pallets** writes the unallocated-container list to `backup.txt` and asks before overwriting an existing file.
- **Restore Pallets** reads `backup.txt` and replaces the current unallocated-container list after confirmation.
- The current backup implementation does not save the pallet assignments; it backs up the unallocated list only.

Keep a copy of `backup.txt` if the data is important. The backup is a simple text format and does not provide encryption or tamper protection.

---

## Build requirements

Install the following:

1. **CMake 3.16 or newer**.
2. A C++ compiler with C++17 support (for example, MSVC on Windows, GCC, or Clang).
3. **Qt 6 or Qt 5** with the modules required by each target:
   - Client: Widgets, Network, Xml.
   - Server: Widgets, Network.
4. A build tool supported by CMake, such as Ninja or Make. Qt Creator can also configure and build the CMake projects.

---

## Build and run

Build each application separately.

### Option A: Qt Creator

1. Open `Container/CMakeLists.txt` in Qt Creator and configure it with a Qt kit that supports C++17.
2. Build and run the `Container` target.
3. Open `ContainerServer/CMakeLists.txt` as a separate project.
4. Configure it with a compatible Qt kit, then build and run the `ContainerServer` target.

### Option B: Command line

Run the following commands from the project root. If CMake cannot locate Qt automatically, set `CMAKE_PREFIX_PATH` to the Qt installation prefix for your compiler and Qt version.

**Build the client:**

```bash
cmake -S Container -B build/Container -DCMAKE_PREFIX_PATH="<path-to-your-Qt-installation>"
cmake --build build/Container --config Release
```

**Build the server:**

```bash
cmake -S ContainerServer -B build/ContainerServer -DCMAKE_PREFIX_PATH="<path-to-your-Qt-installation>"
cmake --build build/ContainerServer --config Release
```

The exact executable location depends on the operating system and generator. With a multi-configuration generator such as Visual Studio, the executable is commonly placed under a `Release` or `Debug` subdirectory.

### Run order

1. Launch `ContainerServer`.
2. Click **Start Listening**. Confirm that its status indicates it is listening on port `6164`.
3. Launch `Container`.
4. Create containers and move them to one or more pallets.
5. Open the **Post XML** tab and select **Post XML to network**.
6. Check the server window for the received container rows.

Both programs should be running on the same computer with the current `127.0.0.1` client configuration.

---

## How to use

### 1. Create containers

- Enter the dimensions and weight for a Box or Cylinder.
- Use the relevant Add action to create it.
- The new container appears in the unallocated list with its generated code, type, weight and volume.

### 2. Assign containers to pallets

- Select a container in the unallocated list.
- Choose a pallet number.
- Select **Move to pallet**.
- Repeat for other containers. Containers assigned to a pallet are removed from the unallocated list.

### 3. Back up or restore unallocated containers

- Use **Backup Pallets** to write the unallocated containers to `backup.txt`.
- Use **Restore Pallets** to replace the current unallocated list with the saved data.

Despite the menu wording, the current implementation backs up/restores unallocated containers, not the pallet map.

### 4. Send pallet data

- Start the server listener first.
- In the client, open **Post XML**.
- Select **Post XML to network**.
- The generated XML is shown in the client tab and sent to the server.
- The server parses the message and displays the received records.

---

## Important implementation notes

These notes describe the supplied source code, rather than assumptions about intended behaviour:

- The client connects to `127.0.0.1:6164`; remote-machine configuration is not exposed in the UI.
- The server listens on `QHostAddress::Any` at port `6164` and stores one active client socket pointer.
- The XML worker runs on a separate `QThread`; the socket send is initiated after XML generation completes.
- The server clears the table when listening is started and also clears existing rows when parsing a new XML document.
- The backup file contains unallocated containers only, not the pallet assignments or the complete application state.
- The code uses raw pointers for containers and pallets, with ownership handled by destructors and explicit cleanup in the current implementation.
- The server's code check validates the code's shape using a regular expression. It does not authenticate the sender or validate all numeric values and dimensions.
- The TCP receiver reads available data and then processes it as one XML document. The current protocol does not define a separate message-length header or a framing delimiter. For larger payloads or unreliable network conditions, a more explicit framing strategy and buffered reads would be a useful improvement.
- The client displays a success dialog from its socket's `disconnected` signal. For production-grade delivery confirmation, the server should send an application-level acknowledgement after successful parsing rather than relying only on the TCP connection closing.

---

## Possible future improvements

The following are suggestions, not features currently implemented:

- Add configurable server host and port settings so the client can connect to another computer.
- Add a server acknowledgement message so the client can distinguish successful parsing from a successful TCP write.
- Introduce robust TCP message framing and buffered receive logic.
- Support multiple simultaneous clients with a socket collection rather than a single active socket pointer.
- Validate numeric fields, dimensions, required XML elements and duplicate container codes.
- Persist complete pallet assignments as well as unallocated containers.
- Allow the user to choose the backup file location and use a versioned, structured backup format.
- Add automated unit tests for volume calculations, code generation, backup/restore, XML serialization and XML parsing.
- Consider smart pointers and clearer ownership rules to reduce manual memory management.
- Add a formal XML schema or documented protocol version to make client/server compatibility easier to maintain.

---

## License

No license file was included in the supplied project archive. Add a `LICENSE` file before redistributing the project if you want to specify terms for reuse.
