---
layout: default
title: 🦎 Pareto Fronts and Archives / C++ and Python 📉🤔
nav_order: 1
has_children: false
has_toc: false
---
# 🦎 Pareto Fronts and Archives / C++ and Python 📉🤔

[![Build Status](https://img.shields.io/github/workflow/status/alandefreitas/pareto-front/ParetoFront?event=push&label=Build&logo=Github-Actions)](https://github.com/alandefreitas/pareto-front/actions?query=workflow%3AParetoFront+event%3Apush)
[![Latest Release](https://img.shields.io/github/release/alandefreitas/pareto-front.svg?label=Download)](https://GitHub.com/alandefreitas/pareto-front/releases/)
[![Website lbesson.bitbucket.org](https://img.shields.io/website-up-down-green-red/http/alandefreitas.github.io/pareto-front.svg?label=Documentation)](https://alandefreitas.github.io/pareto-front/)

![Two-dimensional front](img/front2d_b.svg)

Containers based on Pareto efficiency simultaneously consider many criteria where we cannot improve one criterion without making at least one other criterion worse off. New elements can either replace elements that are worse at all criteria or send them to a higher-level archive front. While there are many libraries for multi-objective optimization, there are no libraries focused on efficient container types for storing these fronts in general applications.
This library provides a STL-like container representing a data structure to cache and query multi-dimensional Pareto fronts and archives with its most expensive operations in <img src="https://render.githubusercontent.com/render/math?math=O(n+\log+n)"> time.

<!-- https://github.com/bradvin/social-share-urls -->
[![Facebook](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Facebook&logo=facebook)](https://www.facebook.com/sharer/sharer.php?t=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&u=https://github.com/alandefreitas/pareto-front/)
[![QZone](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+QZone&logo=qzone)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&summary=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Weibo](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Weibo&logo=sina-weibo)](http://sns.qzone.qq.com/cgi-bin/qzshare/cgi_qzshare_onekey?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&summary=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Reddit](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Reddit&logo=reddit)](http://www.reddit.com/submit?url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Twitter](https://img.shields.io/twitter/url/http/shields.io.svg?label=Share+on+Twitter&style=social)](https://twitter.com/intent/tweet?text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python&url=https://github.com/alandefreitas/pareto-front/&hashtags=MOO,MultiObjectiveOptimization,Cpp,ScientificComputing,Optimization,Developers)
[![LinkedIn](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+LinkedIn&logo=linkedin)](https://www.linkedin.com/shareArticle?mini=false&url=https://github.com/alandefreitas/pareto-front/&title=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![WhatsApp](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+WhatsApp&logo=whatsapp)](https://api.whatsapp.com/send?text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python:+https://github.com/alandefreitas/pareto-front/)
[![Line.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Line.me&logo=line)](https://lineit.line.me/share/ui?url=https://github.com/alandefreitas/pareto-front/&text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![Telegram.me](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+Telegram.me&logo=telegram)](https://telegram.me/share/url?url=https://github.com/alandefreitas/pareto-front/&text=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)
[![HackerNews](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Share+on+HackerNews&logo=y-combinator)](https://news.ycombinator.com/submitlink?u=https://github.com/alandefreitas/pareto-front/&t=Pareto%20Fronts%20and%20Archives%20/%20C%2B%2B%20and%20Python)



- [Use cases](use-cases.md)
- [Design goals](design-goals.md)
- [Examples](examples.md)
  - [Constructing fronts](examples/constructing-fronts.md)
  - [Insertion](examples/insertion.md)
  - [Reference points](examples/reference-points.md)
  - [Iterators](examples/iterators.md)
  - [Queries](examples/queries.md)
  - [Dominance](examples/dominance.md)
  - [Indicators](examples/indicators.md)
  - [Archives](examples/archives.md)
- [Functions](functions.md)
- [Benchmarks](benchmarks.md)
- [Integration](integration.md)
  - [Packages](integration/packages.md)
  - [Build from source](integration/build-from-source.md)
    - [Dependencies](integration/build-from-source/dependencies.md)
    - [Build the Examples](integration/build-from-source/build-the-examples.md)
    - [Installing pareto-front from Source](integration/build-from-source/installing-pareto-front-from-source.md)
    - [Building the packages](integration/build-from-source/building-the-packages.md)
  - [CMake targets](integration/cmake-targets.md)
    - [Find it as a CMake Package](integration/cmake-targets/find-it-as-a-cmake-package.md)
    - [Use it as a CMake subdirectory](integration/cmake-targets/use-it-as-a-cmake-subdirectory.md)
    - [CMake with Automatic Download](integration/cmake-targets/cmake-with-automatic-download.md)
  - [Other build systems](integration/other-build-systems.md)
- [Limitations](limitations.md)
- [Contributing](contributing.md)
- [Thanks](thanks.md)
- [References](references.md)


<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
