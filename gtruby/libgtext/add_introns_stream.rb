#
# Copyright (c) 2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
# Copyright (c) 2008 Center for Bioinformatics, University of Hamburg
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

require 'gtdlload'
require 'libgtext/genome_stream'

module GT
  extend DL::Importable
  gtdlload "libgt"
  extern "GenomeStream* add_introns_stream_new(GenomeStream*)"

  class AddIntronsStream
    include GT::GenomeStream
    attr_reader :genome_stream
    def initialize(in_stream)
      @genome_stream = GT.add_introns_stream_new(in_stream.genome_stream)
      @genome_stream.free = GT::symbol("genome_stream_delete", "0P")
    end
  end
end
