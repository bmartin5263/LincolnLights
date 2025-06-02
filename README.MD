# Lincoln Lights

## Phase 1 Goals

- LED ring to replace the clock
  - Arduino will be placed somewhere for easy access to load programs (OTA not required)
- Arduino and LEDs will be powered by OBD-II adapter (5V)
  - Advantage - Simpler setup, no need to convert to 5V for the LEDs
  - Disadvantage - Need to unplug to preserve power
    - Investigate how much power the arduino draws when "idle"
- Time between Arduino startup to the LEDs turning on should be low (maybe 1 second max)
  - Systems that take a noticeable amount of time to startup need to be offloaded to a worker thread
  - WIFI startup takes about 2 seconds
- Systems must be resilient to failure
  - For example, if connection to OBD-II severed, it should not crash. It should simply try to reestablish connection
- Programmable Startup Animation
- Multiple Scenes with ability to arbitrarily cycle between them
- Use Vehicle Speed and Engine Speed for effects

## Phase 2 Goals

- Use the Lincoln clock's existing buttons for cycling scenes
  - 