## Getting Started
1. Copy `include/ocpp_configuration.def.template` to your include path as `ocpp_configuration.def`.
2. Add or edit entries in the `ocpp_configuration.def` file as needed.
3. Pass in `OCPP_CONFIGURATION_DEFINES=\"ocpp_configuration.def\"` at compile time. Or `ocpp_configuration.def.template` will be used by default.
4. Then, `ocpp_init()`.

See [the examples](examples) for more details.

## Supported Protocol
- [x] OCPP 1.6 Client
  - [x] Core
  - [x] Firmware Management
  - [ ] Local Auth List Management
  - [ ] Reservation
  - [ ] Smart Charging
  - [x] Remote Trigger
  - [x] Security
    - [x] Unsecured Transport with Basic Authentication
    - [x] TLS with Basic Authentication
    - [x] TLS with Client Side Certificates
- [ ] OCPP 1.6 Server
- [ ] OCPP 2.0.1 Client
- [ ] OCPP 2.0.1 Server
- [ ] OCPP 2.1 Client
- [ ] OCPP 2.1 Server
