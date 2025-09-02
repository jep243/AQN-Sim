Quantum Module Documentation
----------------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Model Description
*****************

The source code for the new module lives in the directory ``contrib/quantum-network``.

This module is used to simulate a network for distributed quantum computing.
This network uses Optical Burst Switching in a Fat-tree network. Distributed
quantum operations include teledata and telegate. This simulation does not
simulate quantum states, instead it focuses on the coordination and management
of quantum and classical packets in an optical network.

Design
======

The quantum optical network defines new channels and optical devices to
simulate the network access layer of the network. Examples and simulations use
the standard internet stack. Quantum operations like teledata and
telegate are implemented in the quantum-application using UDP.

Scope and Limitations
=====================

This model is focused on the coordination and timing of distributed quantum
operations. It does not simulate quantum states, and does not define a quantum
specific transport layer. To properly simulate a quantum transport layer
notible changes would need to be made to the internet stack, for simplicity,
quantum operations were approximated using a custom application.

References
==========

This work is part of a ongoing Masters Thesis. Simulations of the AQN used in
the thesis can be found in examples/sim.cc. Support scripts can be found in the
scripts directory.

Usage
*****

Optical devices and channels defined in this module simulate optical burst
switching with some additions for quantum communication. Optical-devices can
be created using the optical-helper in a similar manner to point-to-point
net devices. The quantum application acts like the udp echo application with
the following exceptions: every application is both a server and client, and 
communications are modelled after teledata and telegate. A basic example of the 
quantum application can be found in examples/quantum-network-example.

Helpers
=======

The OpticalHelper allows for the installation of optical-devices and channels,
as well as customization of queues. It follows a similar pattern to
point-to-point. One notible difference is the need to define endpoints.
Endpoints are nodes that can transmit and receive network packages, every other
node simply forwards optical data based on control packets that are sent through
the network.

The QuantumHelper installs the quantum application onto a node. After a
application is installed you will need to use the AddPeer function of the
quantum application to add peers in the network that will be communicated to. 

Output
======

The optical device and channel will have various callbacks for tx, rx, drop
passthrough, collision, etc. The quantum application has various log information,
but also will print to std out at certain events like protcolol completion, 
application finish, etc.

Validation
**********

Tests are run to make sure queuing, timing, collisions, optical channels, etc.
are functioning properly. Tests also exist for time nodes which simulate clock
skew, but this functionality was not used in simulations.
