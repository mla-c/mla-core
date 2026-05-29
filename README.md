# mla-core

`mla-core` is the main repository for the mla-c project. It contains the reusable C/C++ core library, platform integrations, test suites, example application code, and the web UI package used for embedded UI delivery.

## Repository Layout

- **base-lib/** — core library, platform abstractions, and test-support utilities  
  [Documentation](base-lib/readme.md)
- **core-test/** — test suite for core functionality  
  [Documentation](core-test/readme.md)
- **core-exmaple-app/** — reference/example application using mla-c modules  
  [Documentation](core-exmaple-app/readme.md)
- **core-web-ui/** — TypeScript/Preact web UI build package whose output is embedded in the C++ UI module  
  [Documentation](core-web-ui/README.md)

## What This Project Provides

- Cross-platform application/runtime primitives
- Networking, HTTP, RPC, serialization, and lifecycle utilities
- UI infrastructure for both native and web-backed flows
- Built-in test and benchmark support

## Start Here

- For library internals and module-level docs: [base-lib/readme.md](base-lib/readme.md)
- For running/expanding tests: [core-test/readme.md](core-test/readme.md)
- For the web UI build workflow: [core-web-ui/README.md](core-web-ui/README.md)

