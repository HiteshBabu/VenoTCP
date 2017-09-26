#include <linux/mm.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/inet_diag.h>

#include <net/tcp.h>

#define V_PARAM_SHIFT 1
static const int beta = 3 << V_PARAM_SHIFT;

#define MAX_SS 65535U
#define WIN_SZ 10000U

/* Veno variables */
struct veno {
	u8 doing_veno_now;	/* if true, do veno for this rtt */
	u16 cntrtt;		/* # of rtts measured within last rtt */
	u32 minrtt;		/* min of rtts measured within last rtt (in usec) */
	u32 basertt;		/* the min of all Veno rtt measurements seen (in usec) */
	u32 inc;		/* decide whether to increase cwnd */
	u32 diff;		/* calculate the diff rate */
};

static void tcp_veno_init(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	tp->snd_ssthresh = MAX_SS;
}

static void tcp_veno_cwnd_event(struct sock *sk, enum tcp_ca_event event)
{

	tcp_set_ca_state(sk, TCP_CA_Recovery);
	struct tcp_sock *tp = tcp_sk(sk);	
	tp->snd_cwnd = WIN_SZ;
	tp->snd_ssthresh = MAX_SS;
	tp->prior_cwnd = WIN_SZ;
	
}

static void tcp_veno_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	tp->snd_cwnd = WIN_SZ;
	tp->snd_ssthresh = MAX_SS;
	tp->prior_cwnd = WIN_SZ;
}

/* Veno MD phase */
static u32 tcp_veno_ssthresh(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	return tp->snd_ssthresh = MAX_SS;

}

static struct tcp_congestion_ops tcp_veno __read_mostly = {
	.init		= tcp_veno_init,
	.ssthresh	= tcp_veno_ssthresh,
	.cong_avoid	= tcp_veno_cong_avoid,
	.cwnd_event	= tcp_veno_cwnd_event,
	.owner		= THIS_MODULE,
	.name		= "veno2",
};

static int __init tcp_veno_register(void)
{
	BUILD_BUG_ON(sizeof(struct veno) > ICSK_CA_PRIV_SIZE);
	tcp_register_congestion_control(&tcp_veno);
	return 0;
}

static void __exit tcp_veno_unregister(void)
{
	tcp_unregister_congestion_cont	rol(&tcp_veno);
}

module_init(tcp_veno_register);
module_exit(tcp_veno_unregister);

MODULE_AUTHOR("Hitesh, Sachin, MOS");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCP Veno");
