# Producer-Consumer

<p>
  <img src="https://img.shields.io/pypi/status/Django.svg"/>
  <img src="https://img.shields.io/badge/contributions-welcome-orange.svg"/>
</p>

 

<p><a href="https://en.wikipedia.org/wiki/Producer–consumer_problem">Producer–Consumer</a> problem (<i>also known as the bounded-buffer problem</i>) is a classic example of a multi-process synchronization problem.</p>

<p>The problem describes two processes, the producer and the consumer, who share a common, fixed-size buffer used as a queue. The producer's job is to generate data, put it into the buffer, and start again. At the same time, the consumer is consuming the data, one piece at a time. The problem is to make sure that the producer won't try to add data into the buffer if it's full and that the consumer won't try to remove data from an empty buffer.</p>

</br>

## Usage
```
Usage:
	./sem -m <messages-count>
	./sem -b <buffer-size>
	./sem -m <messages-count> -b <buffer-size>
	./sem -d -m <messages-count> -b <buffer-size>

Verbose:
	--message     <messages-count>.
	--buffer      <buffer-size>.
	--debug       <debug-mode>.

Options:
	-d            Debug mode.
  ```
