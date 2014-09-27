#!/usr/bin/perl

use LWP::UserAgent;
use HTTP::Request;
use Term::ReadLine;

my $term = new Term::ReadLine 'Hopmod Server Shell';
my $prompt = "server> ";
my $OUT = $term->OUT || \*STDOUT;
while ( defined ($_ = $term->readline($prompt)) ) {
	if ($_) { $res = &toserverpipe($_) ;
        $term->addhistory($_);
		warn $@ if $@;
		print $OUT $res, "\n" unless $@;
	}
}

sub toserverpipe {
        my $content = shift;
        my $connection = LWP::UserAgent->new();
        my $post = HTTP::Request->new(POST => "http://127.0.0.1:28788/serverexec");
        $post->content_type("text/x-lua");
        $post->content($content);
	my $response = $connection->request($post);
        return $response->content;

	
}
