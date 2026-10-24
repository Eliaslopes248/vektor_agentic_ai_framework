# Vektor

Vektor is a C++ agentic AI suite intended to bring the core pieces of building
AI applications together in one framework. The goal is to make it practical to
build agents that can work with multiple model providers, use purpose-built
tools, and ground their responses in a project's own knowledge.

Rather than focusing on a single agent or model integration, Vektor is being
designed as a set of composable libraries and clients that can be used together
or independently.

## What Vektor aims to offer

- **Vector database libraries** for storing and managing embedded documents,
  with a common client interface and support for different storage backends.
- **Embedding support** to turn source material into vectors for retrieval.
- **Knowledge-base and RAG systems out of the box**, covering the path from
  adding knowledge to retrieving relevant context for an agent.
- **Agent clients for different providers**, behind a consistent interface
  where practical, so applications can integrate with different model
  ecosystems.
- **VSTL tool sets for agent clients**, giving agents reusable tools to work
  with data and application capabilities.
- **Application-facing layers**, including an API and CLI, to make the suite
  usable from both applications and developer workflows.

The intended result is a flexible foundation for building complete agentic
applications, from persistence and retrieval through model interaction and
tool use.

## Where the project is today

Development is currently focused on the foundational vector-store layer. The
repository contains a C++17 library with a SQLite-backed persistent client.
It defines collections and document data, stores embeddings and JSON metadata,
and provides operations for creating collections, inserting and upserting
documents, fetching records, and deleting documents or collections.

This first layer is establishing the interfaces and persistence groundwork
that the embedding, knowledge-base, RAG, provider-client, and VSTL components
can build on. The project is under active development, and its broader suite
is taking shape around these foundations.

## Repository layout

```text
vektor/
  include/vectordb/   Public vector-store interfaces and data types
  src/vectordb/       SQLite persistence and vector-store implementation
  unittests/          Unit-test project structure
```

## Building

The project uses Make and a C++17 compiler. From the repository root, build the
library and example executable with:

```sh
make -C vektor main
```

The build links against SQLite and uses Boost and nlohmann/json headers.

### Build and run in debug mode

The `drun` target performs a clean debug build and then runs the example
executable:

```sh
make -C vektor drun
```

The debug build defines `DEBUG`; the vector-store defaults use that to keep
debug data separate under `/tmp/vektor/debug/data_store`. The regular default
data directory is `/tmp/vektor/data_store`.

### Build and run unit tests

The unit-test Makefile has a build target named `ut`. Build the test executable
from the repository root with:

```sh
# Build /unittests/build/bin/unittests executable
make -C vektor/unittests ut

# Build and Run target
make -C vektor/unittests ut-run
```

Then run it with:

```sh
./vektor/unittests/build/bin/unittests
```

Once GoogleTest is wired into the test target, GoogleTest's runner can also be
used to select tests, for example:

```sh
./vektor/unittests/build/bin/unittests --gtest_filter='SuiteName.TestName'
```

The unit-test Makefile is currently still configured to obtain Qt 6 Test
compiler and linker flags with `pkg-config`; it will need to be switched to
GoogleTest as the test suite is brought online.

## How the build and test setup fits together

- The main Makefile discovers C++ source files under `vektor/src`, compiles
  them into object files under `vektor/build/objects`, and uses those objects
  to produce the shared library and example executable under `vektor/build`.
- `make -C vektor drun` reuses the main build flow with the `DEBUG` define,
  starts from a clean build directory, and runs the resulting executable.
- The unit-test Makefile is a separate build: it discovers test `.cpp` files
  under `vektor/unittests/src`, compiles them into
  `vektor/unittests/build/objects`, ensures the Vektor library is built, and
  links the test objects with that library and the test framework into
  `vektor/unittests/build/bin/unittests`.
- With GoogleTest, test cases belong in the unit-test sources and are
  registered with GoogleTest. The test executable's runner discovers and
  executes those cases; the Makefile's role is to compile the test sources
  and link GoogleTest and Vektor together.

The test target is still being set up: the current test source includes a
header that is not present in the repository, so the build-and-run commands
above describe the intended workflow rather than a currently passing test
suite.
