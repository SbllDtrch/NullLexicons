library(ggplot2)
library(reshape)
library(lme4)
library(plyr)
library(RColorBrewer)


setwd(.)

StdErr <- function (x) { return (sd(x)/sqrt(length(x))); }


multmerge <- function(mypath)
{
filenames=list.files(path=mypath, full.names=TRUE)
print(filenames)
datalist = lapply(filenames, function(x){read.csv(file=x,header=T)})
Reduce(function(x,y) {rbind(x,y)}, datalist)
}


dat <- list.files('./evaluation')
d <- NULL
for (i in dat) {
	print(paste(dat, i, sep='/'))
    df <- read.csv(paste('./evaluation/', i, sep='/'))
    df$f <- i
    d <- rbind(d, df)
 }

#d <- multmerge('evaluation/all_runs')
#lengths = seq(2, 10, 1)

d$f <- gsub('_syll_', '', d$f)
d$f <- gsub('_pcfg_', '', d$f)
d$f <- gsub('_smoothing0.01', '', d$f)
d$model <- gsub('0.01', '', d$model)
d$f <- gsub('manual_', '', d$f)
d$f <- gsub('\\.txt', '', d$f)
d <- cbind(d, colsplit(d$f, split='_', names = c('global', 'lemma', 'lang', 'mono', 'homo1', 'homo2', 'size', 'cv', 'iter', 'model', 'n')))
d$lang <- mapvalues(d$lang, from = c("dutch", "english", "french", "german"), to = c("Dutch", "English", "French", "German"))


data <- ddply(d, .(model, smoothing, lang), summarize, perplexity = mean(logprob), SE= StdErr(logprob))
data<- subset(data, smoothing == "add")

#pdf('PDFs/all_evaluation.pdf', height = 10, width = 15)

data$model <- mapvalues(data$model, from = c("nphone1", "nphone2","nphone3", "nphone4","nphone5", "nphone6", "nsyll1", "nsyll2", "pcfg1"), to = c("1-phone", "2-phone","3-phone", "4-phone","5-phone", "6-phone", "1-syll", "2-syll", "pcfg"))
data$model <- factor(data$model)
du <- subset(data, lang == "Dutch")
du$model <- reorder(du$model, du$perplexity)
en <- subset(data, lang == "English")
en$model <- reorder(en$model, en$perplexity)
fr <- subset(data, lang == "French")
fr$model <- reorder(fr$model, fr$perplexity)
de <- subset(data, lang == "German")
de$model <- reorder(de$model, de$perplexity)

cmap <- c(brewer.pal(9,"Greens")[4:9], brewer.pal(9,"PiYG")[1],brewer.pal(9,"PiYG")[2], "blue")

quartz()
p <-ggplot(mapping = aes(x = model, y= perplexity, xend= model, yend=-16500)) 
p <- p + geom_segment(data=du, colour="grey50") + geom_segment(data=subset(du, model == "5-phone"), colour="red") + geom_point(data= du, size = 4, colour = cmap) 
p <- p + geom_segment(data=en, colour="grey50") + geom_segment(data=subset(en, model == "5-phone"), colour="red") + geom_point(data= en, size = 4,  colour = cmap)
p <- p + geom_segment(data=fr, colour="grey50")+  geom_segment(data=subset(fr, model == "5-phone"), colour="red")+  geom_point(data= fr, size = 4,  colour = cmap)
p <- p + geom_segment(data=de, colour="grey50")+  geom_segment(data=subset(de, model == "5-phone"), colour="red")+ geom_point(data= de, size = 4,  colour = cmap)
#p <- p +  geom_errorbar(aes(ymin=perplexity-SE, ymax=perplexity+SE), width=.1)
p <- p + ylab("negative logprob")+ xlab("models")+ggtitle("")+ facet_grid(. ~ lang,scales="free_x", space="free_x") + theme_bw()
p <- p + opts(axis.line=theme_blank(),axis.text.x=theme_text(angle=90),axis.title.x=theme_blank())+theme(text = element_text(size=20))
p <- p + ylim(-16500, -4000) 
#p <- p  
#+ geom_point(data=subset(data, smoothing == "add" & model == "nphone5"), size = 6, colour = 'red',fill = 'red' , shape=23)
print(p)
#dev.off()
#}


# data <- read.table("~/Desktop/lexicon/rfiles/lengths.txt", header = T)
# ggplot(data, aes(count, colour = language)) + geom_density(alpha = 0.2)