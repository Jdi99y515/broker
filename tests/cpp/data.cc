#include "broker/broker.hh"

#define SUITE data
#include "test.hpp"

using namespace broker;

TEST(basic) {
  CHECK(std::is_same<boolean, bool>::value);
  CHECK(std::is_same<integer, int64_t>::value);
  CHECK(std::is_same<count, uint64_t>::value);
  CHECK(std::is_same<real, double>::value);
}

TEST(time duration) {
  time::duration d;
  CHECK_EQUAL(d.unit, time::unit::invalid);
  CHECK_EQUAL(d.count, 0);
  d = std::chrono::milliseconds(100);
  CHECK_EQUAL(d.unit, time::unit::milliseconds);
  CHECK_EQUAL(d.count, 100);
  d = std::chrono::nanoseconds(100);
  CHECK_EQUAL(d.unit, time::unit::nanoseconds);
  CHECK_EQUAL(d.count, 100);
  MESSAGE("std::chrono::time_point interoperability");
  std::chrono::system_clock::time_point tp;
  tp += time::duration{time::unit::milliseconds, 10};
  auto since_epoch = tp.time_since_epoch();
  auto us = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch);
  CHECK_EQUAL(us.count(), 10 * 1000);
  MESSAGE("conversion");
  auto fractional = to<double>(d);
  REQUIRE(fractional);
  CHECK_EQUAL(*fractional, 100e-9);
  d = {time::unit::seconds, 42};
  auto ms = to<std::chrono::milliseconds>(d);
  REQUIRE(ms);
  CHECK_EQUAL(ms->count(), 42 * 1000);
  MESSAGE("printing");
  CHECK_EQUAL(to_string(time::duration{}), "0");
  CHECK_EQUAL(to_string(time::duration{time::unit::seconds, -10}), "-10s");
  CHECK_EQUAL(to_string(time::duration{time::unit::nanoseconds, 7}), "7ns");
}

TEST(time point) {
  time::point p;
  CHECK_EQUAL(p.value.count, 0);
}

TEST(address) {
  address a;
  // Default-constructed addresses are considered IPv6.
  CHECK(!a.is_v4());
  CHECK(a.is_v6());
  MESSAGE("parsing");
  auto opt = to<address>("dead::beef");
  REQUIRE(opt);
  CHECK(!opt->is_v4());
  CHECK(opt->is_v6());
  opt = to<address>("1.2.3.4");
  REQUIRE(opt);
  CHECK(opt->is_v4());
  CHECK(!opt->is_v6());
  MESSAGE("printing");
  CHECK_EQUAL(to_string(*opt), "1.2.3.4");
  MESSAGE("masking");
  CHECK(opt->mask(96 + 16));
  CHECK_EQUAL(to_string(*opt), "1.2.0.0");
}

TEST(port) {
  port p;
  CHECK_EQUAL(p.number(), 0u);
  CHECK(p.type() == port::protocol::unknown);
  p = {80, port::protocol::tcp};
  MESSAGE("parsing");
  auto opt = to<port>("8/icmp");
  REQUIRE(opt);
  CHECK_EQUAL(*opt, port(8, port::protocol::icmp));
  opt = to<port>("42/nonsense");
  REQUIRE(opt);
  CHECK_EQUAL(*opt, port(42, port::protocol::unknown));
  MESSAGE("printing");
  CHECK_EQUAL(to_string(p), "80/tcp");
  p = {0, port::protocol::unknown};
  CHECK_EQUAL(to_string(p), "0/?");
}

TEST(subnet) {
  subnet sn;
  CHECK_EQUAL(sn.length(), 0u);
  CHECK_EQUAL(to_string(sn), "::/0");
  auto a = to<address>("1.2.3.4");
  auto b = to<address>("1.2.3.0");
  REQUIRE(a);
  REQUIRE(b);
  sn = {*a, 24};
  CHECK_EQUAL(sn.length(), 24u);
  CHECK_EQUAL(sn.network(), *b);
}
