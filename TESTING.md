# Testing Thunderbolt user-space components

## Overview
To test Thunderbolt `umockdev` package is used. Umockdev mocks Linux devices
to test tools even on platforms without actual Thunderbolt connected.

## Testing inside docker container
To ease setting up environment for the testing there is docker configuration.

### Testing docker locally
- Install docker following instructions: https://docs.docker.com/get-started/#setup
- Create docker image `make docker-build`
- Run tests `make docker-run`

### Testing in Travis CI platform
Check build results and trigger builds can be done here:
https://travis-ci.org/01org/thunderbolt-software-user-space

## Testing locally
- Build and install `umockdev` following instructions here:
https://github.com/martinpitt/umockdev
- Use special makefile target: `make check`
