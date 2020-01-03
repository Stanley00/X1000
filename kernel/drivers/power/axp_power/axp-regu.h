#ifndef _LINUX_AXP_REGU_H_
#define _LINUX_AXP_REGU_H_

#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define AXP19_LDO1_MIN		1250000
#define AXP19_LDO1_MAX		1250000
#define AXP19_LDO2_MIN		1800000
#define AXP19_LDO2_MAX		3300000
#define AXP19_LDO3_MIN		700000
#define AXP19_LDO3_MAX		3500000
#define AXP19_LDO4_MIN		1800000
#define AXP19_LDO4_MAX		3300000
#define AXP19_LDOIO0_MIN	1800000
#define AXP19_LDOIO0_MAX	3300000
#define AXP19_DCDC1_MIN		700000
#define AXP19_DCDC1_MAX		3500000
#define AXP19_DCDC2_MIN		700000
#define AXP19_DCDC2_MAX		2275000
#define AXP19_DCDC3_MIN		700000
#define AXP19_DCDC3_MAX		3500000

#define AXP19_LDO1_STEP		0
#define AXP19_LDO2_STEP		100000
#define AXP19_LDO3_STEP		25000
#define AXP19_LDO4_STEP		100000
#define AXP19_LDOIO0_STEP	100000
#define AXP19_DCDC1_STEP	25000
#define AXP19_DCDC2_STEP	25000
#define AXP19_DCDC3_STEP	25000

#define AXP19_LDO1_SHIFT	0
#define AXP19_LDO2_SHIFT	4
#define AXP19_LDO3_SHIFT	0
#define AXP19_LDO4_SHIFT	0
#define AXP19_LDOIO0_SHIFT	4
#define AXP19_DCDC1_SHIFT	0
#define AXP19_DCDC2_SHIFT	0
#define AXP19_DCDC3_SHIFT	0

#define AXP19_LDO1_NBITS	0
#define AXP19_LDO2_NBITS	4
#define AXP19_LDO3_NBITS	7
#define AXP19_LDO4_NBITS	4
#define AXP19_LDOIO0_NBITS	4
#define AXP19_DCDC1_NBITS	7
#define AXP19_DCDC2_NBITS	6
#define AXP19_DCDC3_NBITS	7

#define AXP19_LDO1_ENBIT	0
#define AXP19_LDO2_ENBIT	2
#define AXP19_LDO3_ENBIT	3
#define AXP19_LDO4_ENBIT	3
#define AXP19_LDOIO0_ENBIT	0
#define AXP19_DCDC1_ENBIT	0
#define AXP19_DCDC2_ENBIT	0
#define AXP19_DCDC3_ENBIT	1

#define AXP20_LDO1_MIN		1300000
#define AXP20_LDO1_MAX		1300000
#define AXP20_LDO2_MIN		1800000
#define AXP20_LDO2_MAX		3300000
#define AXP20_LDO3_MIN		700000
#define AXP20_LDO3_MAX		3500000
#define AXP20_LDO4_MIN		1250000
#define AXP20_LDO4_MAX		3300000
#define AXP20_LDOIO0_MIN	1800000
#define AXP20_LDOIO0_MAX	3300000
#define AXP20_DCDC1_MIN		700000
#define AXP20_DCDC1_MAX		3500000
#define AXP20_DCDC2_MIN		700000
#define AXP20_DCDC2_MAX		2275000
#define AXP20_DCDC3_MIN		700000
#define AXP20_DCDC3_MAX		3500000

#define AXP20_LDO1_STEP		0
#define AXP20_LDO2_STEP		100000
#define AXP20_LDO3_STEP		25000
#define AXP20_LDO4_STEP		130000
#define AXP20_LDOIO0_STEP	100000
#define AXP20_DCDC1_STEP	25000
#define AXP20_DCDC2_STEP	25000
#define AXP20_DCDC3_STEP	25000

#define AXP20_LDO1_SHIFT	0
#define AXP20_LDO2_SHIFT	4
#define AXP20_LDO3_SHIFT	0
#define AXP20_LDO4_SHIFT	0
#define AXP20_LDOIO0_SHIFT	4
#define AXP20_DCDC1_SHIFT	0
#define AXP20_DCDC2_SHIFT	0
#define AXP20_DCDC3_SHIFT	0

#define AXP20_LDO1_NBITS	0
#define AXP20_LDO2_NBITS	4
#define AXP20_LDO3_NBITS	7
#define AXP20_LDO4_NBITS	4
#define AXP20_LDOIO0_NBITS	4
#define AXP20_DCDC1_NBITS	6
#define AXP20_DCDC2_NBITS	6
#define AXP20_DCDC3_NBITS	7

#define AXP20_LDO1_ENBIT	0
#define AXP20_LDO2_ENBIT	2
#define AXP20_LDO3_ENBIT	6
#define AXP20_LDO4_ENBIT	3
#define AXP20_LDOIO0_ENBIT	0
#define AXP20_DCDC1_ENBIT	4
#define AXP20_DCDC2_ENBIT	4
#define AXP20_DCDC3_ENBIT	1

#if defined (CONFIG_KP_AXP19)
#define AXP_LDO1_MIN		AXP19_LDO1_MIN
#define AXP_LDO1_MAX		AXP19_LDO1_MAX
#define AXP_LDO2_MIN		AXP19_LDO2_MIN
#define AXP_LDO2_MAX		AXP19_LDO2_MAX
#define AXP_LDO3_MIN		AXP19_LDO3_MIN
#define AXP_LDO3_MAX		AXP19_LDO3_MAX
#define AXP_LDO4_MIN		AXP19_LDO4_MIN
#define AXP_LDO4_MAX		AXP19_LDO4_MAX
#define AXP_LDOIO0_MIN		AXP19_LDOIO0_MIN
#define AXP_LDOIO0_MAX		AXP19_LDOIO0_MAX
#define AXP_DCDC1_MIN		AXP19_DCDC1_MIN
#define AXP_DCDC1_MAX		AXP19_DCDC1_MAX
#define AXP_DCDC2_MIN		AXP19_DCDC2_MIN
#define AXP_DCDC2_MAX		AXP19_DCDC2_MAX
#define AXP_DCDC3_MIN		AXP19_DCDC3_MIN
#define AXP_DCDC3_MAX		AXP19_DCDC3_MAX
#define AXP_DCDC4_MIN		AXP19_DCDC3_MIN
#define AXP_DCDC4_MAX		AXP19_DCDC3_MAX

#define AXP_LDO1_STEP		AXP19_LDO1_STEP
#define AXP_LDO2_STEP		AXP19_LDO2_STEP
#define AXP_LDO3_STEP		AXP19_LDO3_STEP
#define AXP_LDO4_STEP		AXP19_LDO4_STEP
#define AXP_LDOIO0_STEP		AXP19_LDOIO0_STEP
#define AXP_DCDC1_STEP		AXP19_DCDC1_STEP
#define AXP_DCDC2_STEP		AXP19_DCDC2_STEP
#define AXP_DCDC3_STEP		AXP19_DCDC3_STEP
#define AXP_DCDC4_STEP		AXP19_DCDC3_STEP

#define AXP_LDO1_SHIFT		AXP19_LDO1_SHIFT
#define AXP_LDO2_SHIFT		AXP19_LDO2_SHIFT
#define AXP_LDO3_SHIFT		AXP19_LDO3_SHIFT
#define AXP_LDO4_SHIFT		AXP19_LDO4_SHIFT
#define AXP_LDOIO0_SHIFT	AXP19_LDOIO0_SHIFT
#define AXP_DCDC1_SHIFT		AXP19_DCDC1_SHIFT
#define AXP_DCDC2_SHIFT		AXP19_DCDC2_SHIFT
#define AXP_DCDC3_SHIFT		AXP19_DCDC3_SHIFT
#define AXP_DCDC4_SHIFT		AXP19_DCDC3_SHIFT

#define AXP_LDO1_NBITS		AXP19_LDO1_NBITS
#define AXP_LDO2_NBITS		AXP19_LDO2_NBITS
#define AXP_LDO3_NBITS		AXP19_LDO3_NBITS
#define AXP_LDO4_NBITS		AXP19_LDO4_NBITS
#define AXP_LDOIO0_NBITS	AXP19_LDOIO0_NBITS
#define AXP_DCDC1_NBITS		AXP19_DCDC1_NBITS
#define AXP_DCDC2_NBITS		AXP19_DCDC2_NBITS
#define AXP_DCDC3_NBITS		AXP19_DCDC3_NBITS
#define AXP_DCDC4_NBITS		AXP19_DCDC3_NBITS

#define AXP_LDO1_ENBIT		AXP19_LDO1_ENBIT
#define AXP_LDO2_ENBIT		AXP19_LDO2_ENBIT
#define AXP_LDO3_ENBIT		AXP19_LDO3_ENBIT
#define AXP_LDO4_ENBIT		AXP19_LDO4_ENBIT
#define AXP_LDOIO0_ENBIT	AXP19_LDOIO0_ENBIT
#define AXP_DCDC1_ENBIT		AXP19_DCDC1_ENBIT
#define AXP_DCDC2_ENBIT		AXP19_DCDC2_ENBIT
#define AXP_DCDC3_ENBIT		AXP19_DCDC3_ENBIT
#define AXP_DCDC4_ENBIT		AXP19_DCDC3_ENBIT

#define AXP_LDO1			POWER19_STATUS
#define AXP_LDO2			POWER19_LDO24OUT_VOL
#define AXP_LDO3			POWER19_LDO3OUT_VOL
#define AXP_LDO4			POWER19_LDO24OUT_VOL
#define AXP_BUCK1			POWER19_DC1OUT_VOL		
#define AXP_BUCK2			POWER19_DC2OUT_VOL
#define AXP_BUCK3			POWER19_DC3OUT_VOL
#define AXP_BUCK4			POWER19_DC3OUT_VOL
#define AXP_LDOIO0			POWER19_GPIO0_VOL

#define AXP_LDO1EN			POWER19_STATUS
#define AXP_LDO2EN			POWER19_LDO24_DC13_CTL
#define AXP_LDO3EN			POWER19_LDO3_DC2_CTL
#define AXP_LDO4EN			POWER19_LDO24_DC13_CTL
#define AXP_BUCK1EN			POWER19_LDO24_DC13_CTL
#define AXP_BUCK2EN			POWER19_LDO3_DC2_CTL
#define AXP_BUCK3EN			POWER19_LDO24_DC13_CTL
#define AXP_BUCK4EN			POWER19_LDO24_DC13_CTL
#define AXP_LDOIO0EN		POWER19_GPIO0_CTL

#define AXP_BUCKMODE		POWER19_DCDC_MODESET
#define AXP_BUCKFREQ		POWER19_DCDC_FREQSET
#endif

#if defined (CONFIG_KP_AXP20)

#define AXP_LDO1_MIN		AXP20_LDO1_MIN
#define AXP_LDO1_MAX		AXP20_LDO1_MAX
#define AXP_LDO2_MIN		AXP20_LDO2_MIN
#define AXP_LDO2_MAX		AXP20_LDO2_MAX
#define AXP_LDO3_MIN		AXP20_LDO3_MIN
#define AXP_LDO3_MAX		AXP20_LDO3_MAX
#define AXP_LDO4_MIN		AXP20_LDO4_MIN
#define AXP_LDO4_MAX		AXP20_LDO4_MAX
#define AXP_LDOIO0_MIN		AXP20_LDOIO0_MIN
#define AXP_LDOIO0_MAX		AXP20_LDOIO0_MAX
#define AXP_DCDC1_MIN		AXP20_DCDC1_MIN
#define AXP_DCDC1_MAX		AXP20_DCDC1_MAX
#define AXP_DCDC2_MIN		AXP20_DCDC2_MIN
#define AXP_DCDC2_MAX		AXP20_DCDC2_MAX
#define AXP_DCDC3_MIN		AXP20_DCDC3_MIN
#define AXP_DCDC3_MAX		AXP20_DCDC3_MAX
#define AXP_DCDC4_MIN		AXP20_DCDC3_MIN
#define AXP_DCDC4_MAX		AXP20_DCDC3_MAX

#define AXP_LDO1_STEP		AXP20_LDO1_STEP
#define AXP_LDO2_STEP		AXP20_LDO2_STEP
#define AXP_LDO3_STEP		AXP20_LDO3_STEP
#define AXP_LDO4_STEP		AXP20_LDO4_STEP
#define AXP_LDOIO0_STEP		AXP20_LDOIO0_STEP
#define AXP_DCDC1_STEP		AXP20_DCDC1_STEP
#define AXP_DCDC2_STEP		AXP20_DCDC2_STEP
#define AXP_DCDC3_STEP		AXP20_DCDC3_STEP
#define AXP_DCDC4_STEP		AXP20_DCDC3_STEP

#define AXP_LDO1_SHIFT		AXP20_LDO1_SHIFT
#define AXP_LDO2_SHIFT		AXP20_LDO2_SHIFT
#define AXP_LDO3_SHIFT		AXP20_LDO3_SHIFT
#define AXP_LDO4_SHIFT		AXP20_LDO4_SHIFT
#define AXP_LDOIO0_SHIFT	AXP20_LDOIO0_SHIFT
#define AXP_DCDC1_SHIFT		AXP20_DCDC1_SHIFT
#define AXP_DCDC2_SHIFT		AXP20_DCDC2_SHIFT
#define AXP_DCDC3_SHIFT		AXP20_DCDC3_SHIFT
#define AXP_DCDC4_SHIFT		AXP20_DCDC3_SHIFT

#define AXP_LDO1_NBITS		AXP20_LDO1_NBITS
#define AXP_LDO2_NBITS		AXP20_LDO2_NBITS
#define AXP_LDO3_NBITS		AXP20_LDO3_NBITS
#define AXP_LDO4_NBITS		AXP20_LDO4_NBITS
#define AXP_LDOIO0_NBITS	AXP20_LDOIO0_NBITS
#define AXP_DCDC1_NBITS		AXP20_DCDC1_NBITS
#define AXP_DCDC2_NBITS		AXP20_DCDC2_NBITS
#define AXP_DCDC3_NBITS		AXP20_DCDC3_NBITS
#define AXP_DCDC4_NBITS		AXP20_DCDC3_NBITS

#define AXP_LDO1_ENBIT		AXP20_LDO1_ENBIT
#define AXP_LDO2_ENBIT		AXP20_LDO2_ENBIT
#define AXP_LDO3_ENBIT		AXP20_LDO3_ENBIT
#define AXP_LDO4_ENBIT		AXP20_LDO4_ENBIT
#define AXP_LDOIO0_ENBIT	AXP20_LDOIO0_ENBIT
#define AXP_DCDC1_ENBIT		AXP20_DCDC1_ENBIT
#define AXP_DCDC2_ENBIT		AXP20_DCDC2_ENBIT
#define AXP_DCDC3_ENBIT		AXP20_DCDC3_ENBIT
#define AXP_DCDC4_ENBIT		AXP20_DCDC3_ENBIT

#define AXP_LDO1			POWER20_MODE_CHGSTATUS
#define AXP_LDO2			POWER20_LDO24OUT_VOL
#define AXP_LDO3			POWER20_LDO3OUT_VOL
#define AXP_LDO4			POWER20_LDO24OUT_VOL
#define AXP_BUCK1			POWER20_STATUS		
#define AXP_BUCK2			POWER20_DC2OUT_VOL
#define AXP_BUCK3			POWER20_DC3OUT_VOL
#define AXP_BUCK4			POWER20_DC3OUT_VOL
#define AXP_LDOIO0			POWER20_GPIO0_VOL
                        	
#define AXP_LDO1EN			POWER20_STATUS
#define AXP_LDO2EN			POWER20_LDO234_DC23_CTL
#define AXP_LDO3EN			POWER20_LDO234_DC23_CTL
#define AXP_LDO4EN			POWER20_LDO234_DC23_CTL
#define AXP_BUCK1EN			POWER20_STATUS
#define AXP_BUCK2EN			POWER20_LDO234_DC23_CTL
#define AXP_BUCK3EN			POWER20_LDO234_DC23_CTL
#define AXP_BUCK4EN			POWER20_LDO234_DC23_CTL
#define AXP_LDOIO0EN		POWER20_GPIO0_CTL
                        	
#define AXP_BUCKMODE		POWER20_DCDC_MODESET
#define AXP_BUCKFREQ		POWER20_DCDC_FREQSET

#endif

#define AXP_LDO(_id)		\
{							\
	.desc	= {				\
		.name	= "LDO" #_id,				\
		.type	= REGULATOR_VOLTAGE,		\
		.id	= AXP_ID_LDO##_id,				\
		.n_voltages = (AXP_LDO##_id##_STEP) ? ((AXP_LDO##_id##_MAX - AXP_LDO##_id##_MIN) / (AXP_LDO##_id##_STEP) + 1) : 1,	\
		.owner	= THIS_MODULE,				\
	},										\
	.min_uV		= (AXP_LDO##_id##_MIN),		\
	.max_uV		= (AXP_LDO##_id##_MAX),		\
	.step_uV	= (AXP_LDO##_id##_STEP),	\
	.vol_reg	= (AXP_LDO##_id),			\
	.vol_shift	= (AXP_LDO##_id##_SHIFT),	\
	.vol_nbits	= (AXP_LDO##_id##_NBITS),	\
	.enable_reg	= (AXP_LDO##_id##EN),			\
	.enable_bit	= (AXP_LDO##_id##_ENBIT),	\
}

#define AXP_BUCK(_id)		\
{							\
	.desc	= {				\
		.name	= "DCDC" #_id,				\
		.type	= REGULATOR_VOLTAGE,		\
		.id	= AXP_ID_BUCK##_id,				\
		.n_voltages = (AXP_DCDC##_id##_STEP) ? ((AXP_DCDC##_id##_MAX - AXP_DCDC##_id##_MIN) / (AXP_DCDC##_id##_STEP) + 1) : 1,	\
		.owner	= THIS_MODULE,				\
	},										\
	.min_uV		= (AXP_DCDC##_id##_MIN),		\
	.max_uV		= (AXP_DCDC##_id##_MAX),		\
	.step_uV	= (AXP_DCDC##_id##_STEP),	\
	.vol_reg	= (AXP_BUCK##_id),			\
	.vol_shift	= (AXP_DCDC##_id##_SHIFT),	\
	.vol_nbits	= (AXP_DCDC##_id##_NBITS),	\
	.enable_reg	= (AXP_BUCK##_id##EN),			\
	.enable_bit	= (AXP_DCDC##_id##_ENBIT),	\
}

#define AXP_SW(_pmic, _id, min, max, step, vreg, shift, nbits, ereg, ebit)	\
{									\
	.desc	= {							\
		.name	= "sw"#_id,					\
		.type	= REGULATOR_VOLTAGE,				\
		.id	= AXP_ID_SW##_id,				\
		.n_voltages = (step) ? ((max - min) / step + 1) : 1,	\
		.owner	= THIS_MODULE,					\
	},								\
	.min_uV		= (min) ,					\
	.max_uV		= (max) ,					\
	.step_uV	= (step),				\
	.vol_reg	= AXP_##vreg,				\
	.vol_shift	= (shift),					\
	.vol_nbits	= (nbits),					\
	.enable_reg	= AXP_##ereg,				\
	.enable_bit	= (ebit),					\
}

#define AXP_REGU_ATTR(_name)					\
{									\
	.attr = { .name = #_name,.mode = 0644 },					\
	.show =  _name##_show,				\
	.store = _name##_store, \
}

struct axp_regulator_info {
	struct regulator_desc desc;
	
	int	min_uV;
	int	max_uV;
	int	step_uV;
	int	vol_reg;
	int	vol_shift;
	int	vol_nbits;
	int	enable_reg;
	int	enable_bit;
};

extern void store_axp_dev(struct device * p);
extern void clear_axp_dev(void);
#endif
