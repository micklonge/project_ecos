#ifndef JACK_FSM_H
#define JACK_FSM_H

struct JACK_FSM
{
	int (*handler)(void *);
	int next_state;
};

#define PERFROM_ACTION(jack_fsm, current_state, error_no, data) \
{ \
	error_no = 0;                                            \
	if (jack_fsm[current_state].handler != NULL)             \
	    error_no = jack_fsm[current_state].handler(data);    \
	if (error_no == 0)                                       \
        current_state = jack_fsm[current_state].next_state;  \
}

#endif