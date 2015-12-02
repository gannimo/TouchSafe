# TouchSafe

## Authors
* Mathias Payer <mathias.payer@nebelwelt.net>
* Pramod Jamkhedkar
* Ruby Lee


## Abstract

Unvetted applications  are frequently used to process sensitive data. These applications may leak confidential,
private, or sensitive data, after being given legitimate access to this data. The owner of the data would like
to attach a policy to this data, and have this policy enforced throughout the data's lifetime. However, the
data owner does not have access to source code and is unable to modify third-party applications (or the
operating system), nor does she have the resources for a code review.

Previous approaches to solve this problem typically involve fine-grained information flow tracking via either
(i) additional hardware or (ii) computationally expensive software solutions. In contrast, we propose the
TouchSafe architecture, which leverages a thin virtualization layer to enable the use of unvetted, unmodified
third-party applications on sensitive data, while enforcing initial access control and subsequent post-access
usage and output control.

We demonstrate that TouchSafe (i) supports the secure use of sensitive data by arbitrary, unvetted
applications, and (ii) enforces highly-efficient post-access output control at file-level granularity (per
application), without the need to modify or trust either the application or the operating system. TouchSafe has
low performance overhead for our implementation prototype, less than 0.5\% on SPEC CPU2006.


## Prototype

* Version 0.2, 2015-12-02
* Version 0.1, 2014-07-28
