class Orus < Formula
  desc "Experimental stack-based virtual machine and language"
  homepage "https://github.com/jordyorel/Orus-lang-Stack-based-VM"
  url "https://github.com/jordyorel/Orus-lang-Stack-based-VM/archive/2a3f12ab16607078bcf5652d20c5d26c471872c5.tar.gz"
  sha256 "341c18cb0202fa33b0bd3c690fdb92c2587d671ca121b019c3ec5f92c9060818"
  version "0.3.0"
  license "MIT"
  head "https://github.com/jordyorel/Orus-lang-Stack-based-VM.git", branch: "main"

  def install
    system "make", "orusc"
    bin.install "orusc"
  end

  test do
    (testpath/"hello.orus").write('print("hello brew")')
    assert_equal "hello brew\n", shell_output("#{bin}/orusc #{testpath}/hello.orus")
  end
end
