import unittest

from cpu_measure import Measure


class TestMeasure(unittest.TestCase):

    def setUp(self):
        self.m = Measure()

    def test_measure(self):
        self.assertEqual(self.m.cycles, 0)
        with self.m:
            1 + 1
        cycles = self.m.cycles
        self.assertGreater(cycles, 0)
        with self.m:
            1 + 1
        self.assertGreater(self.m.cycles, cycles,
                           "Successive measures must increment cycles counter")

    def test_enter_context_only_once(self):
        self.m.__enter__()
        with self.assertRaises(RuntimeError):
            self.m.__enter__()


    def test_exit_from_context_only_once(self):
        self.m.__enter__()
        self.m.__exit__(None, None, None)
        with self.assertRaises(RuntimeError):
            self.m.__exit__(None, None, None)


if __name__ == "__main__":
    unittest.main()

