<!-- when editing this file also update https://github.com/lurkydismal/.github/blob/main/CONTRIBUTING.md -->

Contributing to CCCaster
=======================

_tl;dr: be [courteous](CODE_OF_CONDUCT.md), follow the steps below to set up a development environment; if you stick around and contribute, you can [join the team](https://github.com/lurkydismal/CCCaster/wiki/Contributor-access) and get commit access._

Welcome
-------

We invite you to join the CCCaster team, which is made up of volunteers!
There are many ways to contribute, including writing code, filing issues on GitHub, helping people
on our mailing lists, our chat channels, or on Stack Overflow, helping to triage, reproduce, or
fix bugs that people have filed, adding to our documentation,
doing outreach about CCCaster, or helping out in any other way.

We grant commit access (which includes full rights to the issue
database, such as being able to edit labels) to people who have gained
our trust and demonstrated a commitment to CCCaster. For more details
see the [Contributor access](https://github.com/lurkydismal/CCCaster/wiki/Contributor-access)
page on our wiki.

We communicate primarily over GitHub.

Before you get started, we encourage you to read these documents which describe some of our community norms:

1. [Our code of conduct](CODE_OF_CONDUCT.md), which stipulates explicitly
   that everyone must be gracious, respectful, and professional. This
   also documents our conflict resolution policy and encourages people
   to ask questions.

2. [Values](https://github.com/lurkydismal/CCCaster/wiki/Values),
   which talks about what we care most about.

Helping out in the issue database
---------------------------------

Triage is the process of going through bug reports and determining if they are valid, finding out
how to reproduce them, catching duplicate reports, and generally making our issues list
useful for our engineers.

If you want to help us triage, you are very welcome to do so!

1. Read [our code of conduct](CODE_OF_CONDUCT.md), which stipulates explicitly
   that everyone must be gracious, respectful, and professional. If you're helping out
   with triage, you are representing the CCCaster team, and so you want to make sure to
   make a good impression!

2. Help out as described in our wiki: https://github.com/lurkydismal/CCCaster/wiki/Triage
   You won't be able to add labels at first, so instead start by trying to
   do the other steps, e.g. trying to reproduce the problem and asking for people to
   provide enough details that you can reproduce the problem, pointing out duplicates,
   and so on.

3. Familiarize yourself with our
   [issue hygiene](https://github.com/lurkydismal/CCCaster/wiki/Issue-hygiene) wiki page,
   which covers the meanings of some important GitHub labels and
   milestones.

4. Once you've been doing this for a while, someone will invite you to the ccccaster-hackers
   team on GitHub and you'll be able to add labels too. See the
   [contributor access](https://github.com/lurkydismal/CCCaster/wiki/Contributor-access) wiki
   page for details.


Quality Assurance
-----------------

One of the most useful tasks, closely related to triage, is finding and filing bug reports. Testing
beta releases, looking for regressions, creating test cases, adding to our test suites, and
other work along these lines can really drive the quality of the product up. Creating tests
that increase our test coverage, writing tests for issues others have filed, all these tasks
are really valuable contributions to open source projects.

If this interests you, you can jump in and submit bug reports without needing anyone's permission!
We're especially eager for QA testing when we announce a beta release.
See https://github.com/lurkydismal/CCCaster/wiki/Quality-Assurance for more details.

If you want to contribute test cases, you can also submit PRs. See the next section
for how to set up your development environment.


Developing for CCCaster
----------------------

If you would prefer to write code, you may wish to start with our list of [good first contributions](https://github.com/lurkydismal/CCCaster/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+contribution%22).

To develop for CCCaster, you will eventually need to become familiar
with our processes and conventions. This section lists the documents
that describe these methodologies. The following list is ordered: you
are strongly recommended to go through these documents in the order
presented.

1. [Setting up your app development environment](https://github.com/lurkydismal/CCCaster/wiki/Setting-up-the-App-development-environment),
   which describes the steps you need to configure your computer to
   work on CCCaster's app. CCCaster's app mainly uses C++, Rust, and NASM.

2. [Tree hygiene](https://github.com/lurkydismal/CCCaster/wiki/Tree-hygiene),
   which covers how to land a PR, how to do code review, how to
   handle breaking changes, how to handle regressions, and how to
   handle post-commit test failures.

3. [Our style guide](https://github.com/lurkydismal/CCCaster/wiki/Style-guide-for-CCCaster-repo),
   which includes advice for designing APIs for CCCaster, and how to
   format code in the app.

4. [CCCaster design doc template](https://lurkydismal.github.io/template),
   which should be used when proposing a new technical design. This is a good
   practice to do before coding more intricate changes.
   See also our [guidance for writing design docs](https://github.com/lurkydismal/CCCaster/wiki/Design-Documents).

In addition to the documents, there are many pages on [our Wiki](https://github.com/lurkydismal/CCCaster/wiki/).
For a curated list of pages see the sidebar on the wiki's home page.
They are more or less listed in order of importance.


API documentation
-----------------

Another great area to contribute in is sample code and API documentation. As our API docs are integrated into our source code, see the
"developing for CCCaster" section above for a guide on how to set up your developer environment.

To contribute API documentation, an excellent command of the English language is particularly helpful, as is a careful attention to detail.
We have a [whole section in our style guide](https://github.com/lurkydismal/CCCaster/wiki/Style-guide-for-CCCaster-repo#documentation-dartdocs-javadocs-etc)
that you should read before you write API documentation. It includes notes on the "CCCaster Voice", such as our word and grammar conventions.

In general, a really productive way to improve documentation is to use CCCaster and stop any time your have a question: find the answer, then
document the answer where you first looked for it.

We also keep [a list of areas that need better API documentation](https://github.com/lurkydismal/CCCaster/issues?q=is%3Aopen+is%3Aissue+label%3A%22d%3A+api+docs%22+sort%3Areactions-%2B1-desc).
In many cases, we have written down what needs to be said in the relevant issue, we just haven't gotten around to doing it!

We're especially eager to add sample code and diagrams to our API documentation. Diagrams are generated from CCCaster code that
draws to a canvas, and stored in a [special repository](https://github.com/lurkydismal/CCCaster/assets-for-api-docs/#readme). It can be a lot of fun
to create new diagrams for the API docs.


Releases
--------

If you are interested in participating in our release process, which may involve writing release notes and blog posts, coordinating the actual
generation of binaries, updating our release tooling, and other work of that nature, then please contact [lurkydismal@duck.com](mailto:lurkydismal@duck.com).
