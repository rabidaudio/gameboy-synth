package midivoice

const MaxMemory = 16

type Channel uint8  // 0..<16
type Note uint8     // 0..<128
type Velocity uint8 // 0..<128

type MidiEvent struct {
	Channel  Channel
	Note     Note
	Velocity Velocity
}

// TODO: note-off vs 0-velocity note-on messages?

type Manager struct {
	Channel Channel
	states  []result
	queue   queue
}

// we need a fixed-memory FIFO queue of overflow notes
// to assign when a slot is freed, and we need to be
// able to remove elements from the middle of the queue.
// a linked list would make that easy except we want to
// avoid allocations. Since it is very small and used
// relatively infrequently, linear complexity performance
// is acceptable. Since we need to remove elements from
// the middle of the queue, we use a linked list, but
// one of a bounded size.
// To push, we add it to the buffer and increment
// the index. To pop, we search the opposite direction
// around the buffer until we find a non-blank element,
// return that, and blank the slot out. To remove an
// element, we simply loop through all the elements
// blanking out any matching ones. Here, "blank" is
// represented by Velocity=0.
type queue struct {
	items []result
	start *result
}

type result struct {
	Note     Note
	Velocity Velocity
	next     *result
}

func newQueue(size int) queue {
	q := make([]result, size)
	prev := (*result)(nil)
	for i := size - 1; i >= 0; i-- {
		q[i] = result{next: prev}
		prev = &q[i]
	}
	return queue{items: q, start: &q[0]}
}

func (q *queue) add(r result) {
	// TODO: we could save a pointer to the next open slot and avoid the loop
	v := q.start
	for {
		if v.Velocity == 0 {
			// we can put it here
			v.Note, v.Velocity = r.Note, r.Velocity
			return
		}
		if v.next == nil {
			// we're full
			return
		}
		v = v.next
	}
}

func (q *queue) remove(r result) {
	p := (*result)(nil)
	v := q.start
	for {
		if v == nil {
			// somehow we got a release from a note we didn't think was pressed
			return
		}
		if v.Note != r.Note || v.Velocity == 0 {
			// a different queued note, so step through to the next item
			p, v = v, v.next
			continue
		}
		// we found the one to remove.
		// we need to skip this link in the chain,
		// and move it to the end instead
		v.Note, v.Velocity = 0, 0
		if p == nil {
			q.start = v.next
		} else {
			p.next = v.next
		}
		// put this slot at the end of the list
		// TODO: saving the end pointer avoids a loop
		i := v.next
		for i.next != nil {
			i = i.next
		}
		i.next = v
		return
	}
}

func (q *queue) pop() (result, bool) {
	v := q.start
	if v.Velocity == 0 {
		// nothing in the queue
		return *v, false
	}
	for {
		if v.next == nil || v.next.Velocity == 0 {
			r := result{Note: v.Note, Velocity: v.Velocity}
			v.Note, v.Velocity = 0, 0
			return r, true
		}
		v = v.next
	}
}

func NewManager(channel Channel, voices uint) *Manager {
	return &Manager{
		Channel: channel,
		states:  make([]result, voices),
		queue:   newQueue(16),
	}
}

func (m *Manager) Handle(e MidiEvent) {
	if e.Channel != m.Channel {
		return
	}
	// TODO: round-robin available voices instead

	if e.Velocity == 0 {
		// off note
		for voice, state := range m.states {
			if e.Note == state.Note {
				// we got a match, so we want to either replace this voice
				// with the next in the queue, or turn this voice off if
				// the queue is empty

				// TODO: we could pop the empty one off the queue
				if v, ok := m.queue.pop(); ok {
					// replace
					m.states[voice] = v
				} else {
					// turn off
					m.states[voice].Velocity = 0
				}
				return
			}
		}
		// otherwise we let go of a note that's in the queue, so
		// remove that note from the queue
		m.queue.remove(result{Note: e.Note, Velocity: e.Velocity})
		return
	}

	// else on notes
	for voice, state := range m.states {
		if state.Note == e.Note {
			// update the velocity
			m.states[voice].Velocity = e.Velocity
			return
		}
		if state.Velocity > 0 {
			continue // already in use by another note
		}
		// we found an available voice
		m.states[voice] = result{Note: e.Note, Velocity: e.Velocity}
		return // assigned to voice
	}
	// we've exausted the available voices, so steal the oldest voice
	// and add that one to the queue
	// TODO: not always the first voice

	// TODO: could we have one data structure for queue and state, with an array of pointers for
	// active voices?
	m.queue.add(m.states[0])
	m.states[0].Note, m.states[0].Velocity = e.Note, e.Velocity
}

func (m *Manager) State(voice int) (Note, Velocity) {
	state := m.states[voice]
	return state.Note, state.Velocity
}
