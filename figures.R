library(ggplot2)
library(reshape)
library(lme4)
library(plyr)
library(grid)
require(gridExtra)


z.test = function(a, mu, var){
   zeta = (mean(a) - mu) / var
   return(zeta)
}


p.val = function(z){
	return(2*pnorm(-abs(z)))
}


setwd("NullLexicon")

dat <- list.files('./rfiles')
ind <- dat[grep('^Last_ind', dat)]
dat <- dat[grep('^Last_global', dat)]

datrob <- list.files('./rfiles/robustness')
rob <- datrob[grep('^paperglobal', datrob)]



d <- NULL
for (i in dat) {
    df <- read.csv(paste('./rfiles/', i, sep=''))
    df$f <- i
    d <- rbind(d, df)
}

d.rob <- NULL
for (i in rob) {
    df <- read.csv(paste('./rfiles/robustness/', i, sep=''))
    df$f <- i
    d.rob <- rbind(d.rob, df)
}

d.ind <- NULL
for (i in ind) {
    df <- read.csv(paste('./rfiles/', i, sep=''))
    df$f <- i
    df$f <- gsub('_syll_', '', df$f)
    df$f <- gsub('_pcfg_', '', df$f)
    df$f <- gsub('_smoothing0.01', '', df$f)
    df$f <- gsub('manual_', '', df$f)
    df$f <- gsub('\\.txt', '', df$f)
    d.ind <- rbind(d.ind, df)
            }
            



d$f <- gsub('_syll_', '', d$f)
d$f <- gsub('_pcfg_', '', d$f)
d$f <- gsub('_smoothing0.01', '', d$f)
d$f <- gsub('manual_', '', d$f)
d$f <- gsub('\\.txt', '', d$f)
d <- cbind(d, colsplit(d$f, split='_', names = c('version','global', 'lex', 'lemma', 'lang', 'mono', 'homo1', 'homo2', 'size', 'cv', 'iter', 'model', 'n')))
d$model.n <- paste(d$model, d$n, sep='_')
d$lang <- mapvalues(d$lang, from = c("dutch", "english", "french", "german"), to = c("Dutch", "English", "French", "German"))

d.rob$f <- gsub('_syll_', '', d.rob$f)
d.rob$f <- gsub('_pcfg_', '', d.rob$f)
d.rob$f <- gsub('_smoothing0.01', '', d.rob$f)
d.rob$f <- gsub('manual_', '', d.rob$f)
d.rob$f <- gsub('\\.txt', '', d.rob$f)
d.rob <- cbind(d.rob, colsplit(d.rob$f, split='_', names = c('global', 'lex', 'lemma', 'lang', 'mono', 'homo1', 'homo2', 'size', 'cv', 'iter', 'model', 'n')))
d.rob$model.n <- paste(d.rob$model, d.rob$n, sep='_')
d.rob$lang <- mapvalues(d.rob$lang, from = c("dutch", "english", "french", "german"), to = c("Dutch", "English", "French", "German"))

d.ind <- cbind(d.ind, colsplit(d.ind$f, split='_', names = c('version','global', 'lex', 'lemma', 'lang', 'mono', 'homo1', 'homo2', 'size', 'cv', 'iter', 'model', 'n')))
d.ind$model.n <- paste(d.ind$model, d.ind$n, sep='_')
d.ind$lang <- mapvalues(d.ind$lang, from = c("dutch", "english", "french", "german"), to = c("Dutch", "English", "French", "German"))

#data only from best model
d.n5 <-subset(d, n == "n5" & d$lang %in% c("Dutch", "English", "French", "German"))
d.ind.n5 <-subset(d.ind, n == "n5" & d.ind$lang %in% c("Dutch", "English", "French", "German"))

#x <- ddply(d.ind.n5, .(length, lang, lexicon), summarize, n_length = length(length))
#data <- merge(d.ind.n5, x)

### FIGURE 2: mps & FIGURE4: avg_lev
makehist.mps <- function(d, v, title)
{
#alpha <- ifelse(d$model.n, 1, 0.5)
 d$value <- d[ , v]
real <- d[d$lexicon == 'real', ]
sims <- d[d$lexicon != 'real', ]
 vlines <- ddply(sims, .(lang), summarize, z = mean(value) - 1.96*sd(value))
  vlines2<- ddply(sims, .(lang), summarize,  z2 = mean(value) + 1.96*sd(value))

 p <- ggplot(sims, aes(value, fill=lang))
p <- p + geom_histogram(colour= "white", position ='identity', size = .2) + xlab("") + theme(legend.position = 'none')
p <- p + geom_point(data=real, aes(value, 0), size = 5, color = "red")   + ggtitle(title) + geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines2) + theme_bw()
 p <- p + facet_grid(. ~ lang, scales= "free") + opts(legend.position = "none", axis.title.y= theme_blank()) +theme(plot.title=element_text(size=15, vjust=1.5)) 
 if( v== 'avg_lev')
 {
 	p <- p + scale_x_continuous(breaks = seq(4,6,by=0.05))
 }
 p <- p + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12)) 
print(p)
}
quartz()
makehist.mps(d.n5, 'mps', "minimal pairs count")
quartz()
makehist.mps(d.n5, 'avg_lev', "average Levenshtein distance")


#FIGURE 3: minimal pairs by length

makehist.ind <- function(d, v, title)
{
    d <- subset(d, length >=2 & length <= 7)
    d <- ddply(d, .(lexicon, lang, model.n, length), summarize, value = sum(mps))
    #d$v <- d[, v]
    #d <- summarise(d, value=mean(v, na.rm=T))
d$length <- factor(d$length) 
len <- length(levels(d$length)) *  length(levels(d$lang))
vars <- data.frame(expand.grid(levels(d$length), levels(d$lang)))
labels <- c("","***","***","***","","*","***","***","***","***","","","***","***","","*","***","","*","**","*","***","**","")
colnames(vars) <- c("length", "lang")
dat <- data.frame(x = c(rep(12500,6), rep(25000,6), rep(7500,6), rep(3500,6)), y = rep(15, len), vars, labs= labels)
real <- d[d$lexicon == 'real', ]
 sims <- d[d$lexicon != 'real', ]
  vlines <- ddply(sims, .(lang, length), summarize, z = mean(value) - 1.96*sd(value),  z2 = mean(value) + 1.96*sd(value))

p <- ggplot(sims, aes(value, fill=lang)) + theme_bw()
p <- p + geom_histogram(alpha=1, colour="white", size=0.2) + xlab(v) + theme(legend.position = 'none')
p <- p + geom_point(data=real, aes(value, 0.05), size = 3, colour = 'red')   + ggtitle(title)  + facet_grid(length ~ lang, scales = "free") + geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines) + xlab("minimal pairs count")
p <- p + geom_text(aes(x, y, label=labs, group=NULL),data=dat,size= 9)  
p <- p + opts(legend.position = "none", axis.title.y= theme_blank(), axis.title.x= theme_blank()) 
 p <- p + theme(panel.margin = unit(0.5, "lines"), plot.title=element_text(vjust=1.5),text = element_text(size=20)) 
print(p)
}
d.ind.n5$lang <- factor(d.ind.n5$lang)
makehist.ind(d.ind.n5, '', "minimal pairs counts by length")
makehist.ind(d.ind.n5, '', "Average Levenshtein distance by length")

#FIGURE 5: Example phonological network (jpeg)

#FIGURE 6: Networks

#FIGURE 7: NETWORK MEASURES
makehist.net <- function(d, v, title,min,max)
{
d$value <- d[ , v]
d$measure <- "average clustering coefficient"
real <- d[d$lexicon == 'real', ]
 sims <- d[d$lexicon != 'real', ]
#  d$value <- d[ , 'transitivity']
# d$measure <- "transitivity"
# real <- rbind(real1, d[d$lexicon == 'real', ])
# sims <- rbind(sims1, d[d$lexicon != 'real', ])
 vlines <- ddply(sims, .(lang, measure), summarize, z = mean(value) - 1.96*sd(value),  z2 = mean(value) + 1.96*sd(value))
 p <- ggplot(sims, aes(value, fill=lang))
p <- p + geom_histogram(position ='identity', colour= "white", size = .2) 
p <- p + geom_point(data=real, aes(value, 0.1), size = 4, colour = 'red')+ ggtitle(title) + geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines)
p <- p + facet_grid(. ~ lang, scales = "free_x") + theme_bw() + opts(legend.position = "none") 
p <- p + theme(plot.title=element_text(size=15, vjust=1.5)) + ylab("") + xlab("") +xlim(min,max) #+ scale_x_continuous(breaks = seq(min,max,by=0.1))
  p <- p + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12))
     
return(p)
}

plot7a <- makehist.net(d.n5, 'avg_cluster', "average clustering coefficient",0.1,0.25)
plot7b <- makehist.net(d.n5, 'big_comp', "giant component",0.4,0.8)
plot7c <-makehist.net(d.n5, 'transitivity', "transitivity",0.25,0.37)
#pdf('PDFs/Figure7.pdf',width=14,height=10)
grid.arrange(plot7a, plot7b, plot7c)
#dev.off()


#FIGURE 8: ROBUSTNESS
d.rob$model.n <- factor(d.rob$model.n)
d.rob$model.n <- factor(d.rob$model.n,levels(d.rob$model.n)[c(1,3,2)])
d.rob$model.n <- mapvalues(d.rob$model.n, from = c("mnphonesrilm_n5", "mnphonesrilm_n4", "mnphonesrilm_n6"), to = c("5phone", "4phone", "6phone"))

makehist.mps.rob <- function(d, v, title)
{
 d$value <- d[ , v]
real <- d[d$lexicon == 'real', ]
 sims <- d[d$lexicon != 'real', ]
 p <- ggplot(sims, aes(value, fill=lang))
p <- p + geom_histogram(colour= "white", position ='identity', size = .2,  aes(alpha= model.n)) 
p <- p + geom_point(data=real, aes(value, 0), size = 5, color = "red")   + ggtitle(title)  + theme_bw() 
p <- p + facet_grid(. ~ lang, scales= "free")  +theme(plot.title=element_text(size=15, vjust=1.5)) + xlab("")
 if( v== 'avg_lev')
 {
 	p <- p + scale_x_continuous(breaks = seq(4,6,by=0.01))+ opts(legend.position = "none", axis.title.y= theme_blank())
 }
 else
 {
 	p <- p + scale_fill_discrete(guide = FALSE)+ opts(legend.position=c(0.95, 0.85), axis.title.y= theme_blank(), legend.background = element_rect(size=.5, colour = "black"))
 	
 }
 p <- p + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12))
 p <- p + scale_alpha_discrete(name="Model", range = c(0.15, 1))
return(p)
}
plot8a <- makehist.mps.rob(d.rob, 'mps', "minimal pairs count")
plot8b <- makehist.mps.rob(d.rob, 'avg_lev', "average Levenshtein distance")

makehist.net.rob <- function(d, v, title)
{
d$value <- d[ , v]
real <- d[d$lexicon == 'real', ]
 sims <- d[d$lexicon != 'real', ]
 #vlines <- ddply(sims, .(lang, measure), summarize, z = mean(value) - 1.96*sd(value),  z2 = mean(value) + 1.96*sd(value))
p <- ggplot(sims, aes(value, fill=lang))
p <- p + geom_histogram(position ='identity', colour= "white", size = .2, aes(alpha=model.n)) 
p <- p + geom_point(data=real, aes(value, 0.1), size = 4, colour = 'red')   + ggtitle(title)  + theme_bw() #+ geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines)
p <- p + facet_grid(. ~ lang, scales = "free_x") + opts(legend.position = "none")+ scale_x_continuous(breaks = seq(0,1,by=0.01))
p <- p + theme(plot.title=element_text(size=15, vjust=1.5)) + ylab("") + xlab("")
p <- p + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12))
p <- p + scale_alpha_discrete(range = c(0.15, 1))
     
return(p)
}

plot8c <- makehist.net.rob(d.rob, 'avg_cluster', "average clustering coefficient")
plot8d <- makehist.net.rob(d.rob, 'big_comp', "giant component")
plot8e <-makehist.net.rob(d.rob, 'transitivity', "transitivity")
#pdf('PDFs/Figure8.pdf',width=14,height=20)
grid.arrange(plot8a, plot8b, plot8c, plot8d, plot8e, nrow = 5)
#dev.off()


#FIGURE 9: avg feature difference
d.feat <- read.table("./rfiles/mps_count/contrasts.txt", header = TRUE)
### average feature difference for minimal pairs
makehist.feat <- function(d, v, title)
{
d$value <- d[ , 'avg_feature_diff']
 real <- d[d$lexicon == 'real', ]
 sims <- d[d$lexicon != 'real', ]
  vlines <- ddply(sims, .(lang), summarize, z = mean(value) - 1.96*sd(value),  z2 = mean(value) + 1.96*sd(value))
  print(vlines)
 p <- ggplot(sims, aes(value, fill = lang))
p <- p + geom_histogram(colour= "white",position ='identity', size= 0.2) + xlab("") + theme(legend.position = 'right')
p <- p + geom_point(data=real, aes(value, 0), size = 4, color = "red")   + ggtitle(title) + geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines) + theme_bw()
p <- p + opts(legend.position = "none", axis.title.y= theme_blank()) + facet_grid(. ~ lang)+theme(plot.title=element_text(size=15, vjust=1.5))
p <- p  + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12), strip.text.y = element_text(size = 12))
print(p)
}
makehist.feat(d.feat, j, "average number of features that differ in the in the 5% most frequent minimal pair contrasts")



#FIGURE 10: confusability
library(clue)
conf <- list.files('./rfiles/conf')
d.conf <- NULL
for (i in conf) {
    df <- read.csv(paste('./rfiles/conf/', i, sep=''))
    df$f <- i
    d.conf <- rbind(d.conf, df)
}
data("Phonemes")
d.conf <- cbind(d.conf, colsplit(d.conf$f, split='_', names = c('lang', 'mps', 'c')))
d.conf <- cbind(d.conf, colsplit(d.conf$pairs, split='_', names = c('ph1', 'ph2')))
phones <- c("p", "t", "k", "f", "D", "s", "S","b","d", "g", "v", "T", "z", "Z", "m", "n")
mat <- matrix(Phonemes, ncol=16, nrow=16, dimnames=list(phones, name = phones))
conf <- as.data.frame(as.table(mat))
names(conf) <- c("ph1", "ph2", "conf")
d2 <- merge(subset(d.conf, lang == "english"), conf)
d2$pairs <- factor(d2$pairs)
d2 <-subset(d2 , ph1 %in% phones & ph2 %in% phones)
scores <- NULL
sub <- subset(d2, lang == "english")
sub$pairs <- factor(sub$pairs)
for (p in levels(sub$pairs))
{
	sims <- subset(sub, lexicon != "real" & pairs == p)
	real <- subset(sub, lexicon == "real" & pairs == p)
	if(nrow(real) > 0)
	{
	print(real)
	z <- z.test(real$count, mean(sims$count), var(sims$count))
	scores <- rbind(scores, data.frame(p, z, real$count, real$conf))
	}
}
quartz()
names(scores) <- c("pairs", "z", "count", "conf")
scores <- subset(scores, z != "NaN" & z != "Inf")
p <- ggplot(scores, aes(x= log(conf), y= z)) + geom_point() + geom_smooth(method='lm',formula=y~x) 
p <-  p + theme_bw() +ylab("z-score")+xlab("confusability score (log)")
plot(p)
  
  
  

#FIGURE 11: POS

makehist.pos <- function(d, v, title)
{
d$value <- d[ , "samepos"]
 d$measure <- "within syntactic cat."
real1 <- d[d$lexicon == 'real', ]
 sims1 <- d[d$lexicon != 'real', ]
 print(real1)
  d$value <- d[ , "diffpos"]
 d$measure <- "across syntactic cat."
real <- rbind(real1, d[d$lexicon == 'real', ])
 sims <- rbind(sims1, d[d$lexicon != 'real', ])
  vlines <- ddply(sims, .(lang, measure), summarize, z = mean(value) - 1.96*sd(value),  z2 = mean(value) + 1.96*sd(value))
 p <- ggplot(sims, aes(value, fill = lang))
p <- p + geom_histogram(colour= "white",position ='identity', size = 0.2) + xlab("") + theme(legend.position = 'right')
p <- p + geom_point(data=real, aes(value, 0), size = 4, color = "red")   
p <- p + geom_vline(aes(xintercept=z), linetype = 'longdash', data=vlines) + geom_vline(aes(xintercept=z2), linetype = 'longdash', data=vlines) 
p <- p + theme_bw() + ggtitle(title) + opts(legend.position = "none", axis.title.y= theme_blank()) + facet_grid(measure ~ lang, scales= "free_x")+theme(plot.title=element_text(size=15, vjust=1.5))
 p <- p + theme(panel.margin = unit(1, "lines"), strip.text.x = element_text(size = 12), strip.text.y = element_text(size = 12))
print(p)
}

d.n5$samepos <- d.n5$mps - d.n5$diffpos
#pdf('PDFs/Figure12bis.pdf',width=14)
makehist.pos(subset(d.n5, n == "n5"), j, "minimal pairs count (across vs. within syntactic categories)")
#dev.off()



#FIGURE 12: POS (STEM)
d.stem <- subset(d.rob, (lang == "English" | lang == "germanSTEM" | lang == "dutchSTEM" | lang == "frenchSTEM") & n == "n5")
d.stem$lang <- mapvalues(d.stem$lang, from = c("dutchSTEM","frenchSTEM", "germanSTEM"), to = c("Dutch (roots)", "French (roots)", "German (roots)"))
d.stem$samepos <- d.stem$mps - d.stem$diffpos
#pdf('PDFs/Figure12.pdf',width=14)
makehist.pos(d.stem, j, "minimal pairs count (across vs. within syntactic categories)")
#dev.off()



#Table 1 for latex
maketable <- function(data, var)
{
	print("\\bottomrule")
	real <- "&real"
	sim <- "&$\\mu$ (simulated)"
	sim_sd <- paste("BLAH","&$\\sigma$ (simulated)",sep="")
	z <- "&$z$"
	p <- "&$p$"
	data$lang <- factor(data$lang)
	for(l in levels(data$lang))
	{
		m <- sum(subset(data, lang == l & lexicon == "real")$mps)
		real <- paste(real, round(m,4), sep = "&")	
		d <- ddply(subset(data, lang == l & lexicon != "real"), .(lexicon), summarize, mu = sum(mps))
		sim <- paste(sim, round(mean(d$mu),4), sep = "&")	
		sim_sd <- paste(sim_sd, round(sd(d$mu),4), sep = "&")	
		z_val <- z.test(m, mean(d$mu), sd(d$mu))
		z <- paste(z, round(z_val,1), sep = "&")	
		p <- p.val(z_val)#paste(p, round(p.val(z_val),3), sep = "&")	
		
	}
	print(paste(real, "\\tabularnewline", sep=""))
	print(paste(sim, "\\tabularnewline", sep=""))
	print(paste(sim_sd, "\\tabularnewline", sep=""))
	print(paste(z, "\\tabularnewline", sep=""))
	print(paste(p, "\\tabularnewline", sep=""))
}


maketable(d.n5, "mps")






